#ifndef IIO_BPM_KCOMPAT_H
#define IIO_BPM_KCOMPAT_H
/* Backports of various kernel API to build against older versions.
 */

#include <linux/version.h>

#include <linux/uio_driver.h>

#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>
#include <linux/iio/buffer-dmaengine.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void devm_iio_device_unreg(struct device *dev, void *res)
{
    iio_device_unregister(*(struct iio_dev **)res);
}

#define devm_iio_device_register(dev, indio_dev)                               \
    __devm_iio_device_register((dev), (indio_dev), THIS_MODULE)
static inline int __devm_iio_device_register(struct device *dev,
                         struct iio_dev *indio_dev,
                         struct module *this_mod)
{
    struct iio_dev **ptr;
    int ret;

    ptr = devres_alloc(devm_iio_device_unreg, sizeof(*ptr), GFP_KERNEL);
    if (!ptr)
        return -ENOMEM;

    *ptr = indio_dev;
    ret = iio_device_register(indio_dev);
    if (!ret)
        devres_add(dev, ptr);
    else
        devres_free(ptr);

    return ret;
}
#endif // < 4.15

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0)
#define devm_uio_register_device(parent, info)                                 \
    __devm_uio_register_device(THIS_MODULE, parent, info)

static void devm_uio_unregister_device(struct device *dev, void *res)
{
    uio_unregister_device(*(struct uio_info **)res);
}

static inline int __must_check __devm_uio_register_device(struct module *owner,
                              struct device *parent,
                              struct uio_info *info)
{
    struct uio_info **ptr;
    int ret;

    ptr = devres_alloc(devm_uio_unregister_device, sizeof(*ptr),
               GFP_KERNEL);
    if (!ptr)
        return -ENOMEM;

    *ptr = info;
    ret = __uio_register_device(owner, parent, info);
    if (ret) {
        devres_free(ptr);
        return ret;
    }

    devres_add(parent, ptr);

    return 0;
}
#endif // < 5.7

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
static void __devm_iio_dmaengine_buffer_free(struct device *dev, void *res)
{
    iio_dmaengine_buffer_free(*(struct iio_buffer **)res);
}
static inline struct iio_buffer *
devm_iio_dmaengine_buffer_alloc(struct device *dev, const char *channel)
{
    struct iio_buffer **bufferp, *buffer;

    bufferp = devres_alloc(__devm_iio_dmaengine_buffer_free,
                   sizeof(*bufferp), GFP_KERNEL);
    if (!bufferp)
        return ERR_PTR(-ENOMEM);

    buffer = iio_dmaengine_buffer_alloc(dev, channel);
    if (IS_ERR(buffer)) {
        devres_free(bufferp);
        return buffer;
    }

    *bufferp = buffer;
    devres_add(dev, bufferp);

    return buffer;
}
#endif // < 5.8

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
#define iio_device_alloc(parent, size) iio_device_alloc(size)
#endif // < 5.9

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 13, 0)
#define iio_device_attach_buffer(idev, buf)                                    \
    ({                                                                     \
        iio_device_attach_buffer(idev, buf);                           \
        0;                                                             \
    })

static inline int devm_iio_dmaengine_buffer_setup(struct device *dev,
                          struct iio_dev *indio_dev,
                          const char *channel)
{
    struct iio_buffer *buffer;

    buffer =
        devm_iio_dmaengine_buffer_alloc(indio_dev->dev.parent, channel);
    if (IS_ERR(buffer))
        return PTR_ERR(buffer);

    indio_dev->modes |= INDIO_BUFFER_HARDWARE;

    return iio_device_attach_buffer(indio_dev, buffer);
}
#endif // < 5.14

#endif /* IIO_BPM_KCOMPAT_H */
