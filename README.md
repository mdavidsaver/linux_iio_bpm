# Linux Drivers to NSLS2 Diagnostics Development

The `iio_bpm_dma` module is a [Linux IIO](https://www.kernel.org/doc/html/latest/driver-api/iio/index.html)
driver which exposes an Analog Device [AXI-DMAC](https://wiki.analog.com/resources/tools-software/linux-drivers/axi-dmac)
DMA engine to userspace.

The `pdev_bpm_mmio` module is a Linux Userspace I/O (UIO) driver
which exposes certain memory ranges for userspace access.
Currently this is includes Xilinx Block RAM (BRAM) ranges.

Required Linux kernel configuration parameters (`y` or `m`)

```
CONFIG_IIO_BUFFER_DMAENGINE=y
```

Note that until Linux 5.10 selecting this option could not be selected
through `make menuconfig`, and requires manually appending `.config`
followed by a re-configure (eg. run 'make menuconfig' and save some change).

## Important Notes

The FPGA bitstream must be loaded prior to loading `dma-axi-dmac` module to
avoid a hard system lockup.

Application with a dynamic device tree overlay is known is **not possible**
with the xilinx-2017.4 Linux kernel fork (of Linux 4.8) due to observed
issues with `__local_fixups__` being ignored.

For such systems, it is necessary to either edit the original `.dts` file,
or apply the overlay manually prior to loading with `fdtoverlay`.  eg.

```
$ dtc -@ -Hboth -I dts -O dtb -o system-user.dtbo system-user.dtsi
$ fdtoverlay -i system.dtb -o system-patched.dtb system-user.dtbo
```

Then boot with the resulting `system-patched.dtb`.


## Device Tree

The `iio_bpm_dma` module is instantiated in conjunction with an AXI-DMAC instance.

Note: currently the DMA stream sample format is hard coded as 4x uint32 in CPU byte order.

```
            rx_dma: dma@a0010000 {
                compatible = "adi,axi-dmac-1.00.a";
                #dma-cells = <1>;
...
            };

            bpm: bpm@0 {
                compatible = "bnl,bpm-adma";
                dmas = <&rx_dma 0>;
                dma-names = "rx";
            };
```

## Testing UIO

The `mcat.py` script will `mmap()` a UIO device, then read and print the entire range.

```sh
$ devmem 0xa1000000 32 0xdeadbeef
$ python mcat.py uio0 | hexdump -C
00000000  ef be ad de 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00080000
```


## Testing AXI-DMAC

The `iio-stream.py` script enable/start the DMA channel and begin reading
samples.

```sh
python iio-stream.py iio:device1 voltage0
b'\x00\x00\x00\x00\x...
...
```
