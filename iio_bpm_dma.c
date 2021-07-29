// SPDX-License-Identifier: GPL-2.0-only
/* Expose ADMA engine to userspace
 */
/* (C) 2021 Michael Davidsaver <mdavidsaver@gmail.com>
 */

//#define DEBUG

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/of.h>
#include <linux/of_platform.h>
//#include <linux/of_address.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/events.h>
#include <linux/iio/buffer.h>
#include <linux/iio/kfifo_buf.h>
#include <linux/iio/buffer-dmaengine.h>

#include "kcompat.h"

#define DRIVER_NAME "iio_bpm_dma"
MODULE_AUTHOR("Michael Davidsaver <mdavidsaver@gmail.com>");
MODULE_DESCRIPTION("Expose ADMA engine to userspace");
MODULE_LICENSE("GPL");

#define bpm_dbg(pdev, fmt, ...)                                                \
    dev_dbg(&pdev->idev->dev, "%s() " fmt, __func__, ##__VA_ARGS__)

struct iio_bpm_device {
    struct iio_dev *idev;
    struct iio_buffer *buffer;
    struct device *dma;
};

#define BPM_SCAN 0
static const struct iio_chan_spec iio_bpm_spec[] = { {
    .type = IIO_VOLTAGE,
    .indexed = 1,
    .channel = 0,
    .scan_index = BPM_SCAN,
    .scan_type = {
         // 4x 32 bit unsigned = 16 bytes per scan
         .endianness = IIO_CPU,
         .sign = 'u',
         .realbits = 32u,
         .storagebits = 32u,
         .repeat = 4u,
     },
} };

static const struct iio_info iio_bpm_info = {};

static int iio_bpm_dma_probe(struct platform_device *pdev)
{
    int ret = -EINVAL;
    struct iio_dev *idev;
    struct iio_bpm_device *bdev;

    if (!(idev = devm_iio_device_alloc(&pdev->dev, sizeof(*bdev)))) {
        ret = -ENOMEM;
        goto err;
    }

    idev->name = "bpm-adma";
    idev->channels = iio_bpm_spec;
    idev->num_channels = ARRAY_SIZE(iio_bpm_spec);
    idev->info = &iio_bpm_info;
    //    idev->modes = INDIO_DIRECT_MODE | INDIO_BUFFER_SOFTWARE;
    idev->dev.parent = &pdev->dev;

    bdev = iio_priv(idev);
    bdev->idev = idev;

    if (!!(ret = devm_iio_dmaengine_buffer_setup(&pdev->dev, idev, "rx"))) {
        dev_err(&pdev->dev, "Failed to setup DMA engine: %d\n", ret);
        goto err;
    }

    ret = devm_iio_device_register(&pdev->dev, idev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to register IIO: %d\n", ret);
        goto err;
    }

    return 0;
err:
    return ret;
}

/* bpm_dma@0 {
 *     compatible = "bnl,bpm-adma";
 *     dmas = <&dma1 0>;
 *     dma-names = "rx";
 * }
 *
 * Where dma= references an AXI-DMA device instance.
 * https://wiki.analog.com/resources/tools-software/linux-drivers/axi-dmac
 */

static struct of_device_id iio_bpm_dma_match[] = {
    { .compatible = "bnl,bpm-adma" },
    {},
};

static struct platform_driver bpm_devices = {
    .probe = iio_bpm_dma_probe,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = of_match_ptr(iio_bpm_dma_match),
    },
};

module_platform_driver(bpm_devices);
