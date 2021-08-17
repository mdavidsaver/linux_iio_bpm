// SPDX-License-Identifier: GPL-2.0-only
/* Expose some simple platform devices w/ MMIO
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

#include <linux/uio_driver.h>

#include "kcompat.h"

#define DRIVER_NAME "pdev_bpm_mmio"
MODULE_AUTHOR("Michael Davidsaver <mdavidsaver@gmail.com>");
MODULE_DESCRIPTION("NSLS2 BPM Platform Device Adapter");
MODULE_LICENSE("GPL");

#ifndef CONFIG_OF
#warning Missing CONFIG_OF
#endif

#define bpm_dbg(pdev, fmt, ...)                                                \
    dev_dbg(&pdev->dev, "%s() " fmt, __func__, ##__VA_ARGS__)

struct bpm_mmio_device {
    struct uio_info info;
    struct platform_device *pdev;
};

static int pdev_bpm_mmio_probe(struct platform_device *pdev)
{
    int i;
    int ret = -EINVAL;
    struct bpm_mmio_device *bdev =
        devm_kzalloc(&pdev->dev, sizeof(*bdev), GFP_KERNEL);

    if (!bdev) {
        ret = -ENOMEM;
        goto err;
    }

    for (i = 0; i < pdev->num_resources; ++i) {
        if (i >= MAX_UIO_MAPS)
            break;

        bdev->info.mem[i].name = "RAM";

        if (pdev->resource[i].flags == IORESOURCE_MEM) {
            bdev->info.mem[i].memtype = UIO_MEM_PHYS;
            bdev->info.mem[i].addr = pdev->resource[i].start;
            bdev->info.mem[i].size =
                resource_size(&pdev->resource[i]);
            /* don't need to make internal_addr */

        } else {
            bdev->info.mem[i].memtype = UIO_MEM_NONE;
            bdev->info.mem[i].size =
                1; /* Otherwise UIO will stop searching... */
        }
    }

    bdev->info.name = "BPM";
    bdev->info.version = "0";
    bdev->info.priv = bdev;

    ret = devm_uio_register_device(&pdev->dev, &bdev->info);
    if (ret)
        goto err;

    return 0;
err:
    bpm_dbg(pdev, "probe error: %d\n", ret);
    return ret;
}

static struct of_device_id pdev_bpm_mmio_match[] = {
    { .compatible = "xlnx,axi-bram-ctrl-4.0" },
    { .compatible = "xlnx,axi-bram-ctrl-4.1" },
    { .compatible = "xlnx,DDRInterface-ip-1.0" },
    { .compatible = "xlnx,up-axi-1.0" },
    { .compatible = "xlnx,sgdma-p2h-wrapper-1.0" },
    {},
};

static struct platform_driver bpm_devices = {
    .probe = pdev_bpm_mmio_probe,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = of_match_ptr(pdev_bpm_mmio_match),
    },
};

module_platform_driver(bpm_devices);
