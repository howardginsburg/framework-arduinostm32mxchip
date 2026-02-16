# Building A Standalone Binary

The DevKit [FAQ](https://microsoft.github.io/azure-iot-developer-kit/docs/faq/) provides some general guidance about how to get the boot.bin and python script to merge the binaries together.  I found that the latest version of python required a tweak to the script to make it work.

## Building the Binary

1. Copy all the files in this directory to a temp folder.
1. Copy the `firmware.bin` file from the `.pio/build/<platformioenv>/` directory to the temp folder.
1. Open a WSL session in VSCode.
1. Run the `buildbinary.sh` script to generate the standalone binary.

```bash
./buildbinary.sh
```

1. The output binary, patched_firmware.bin will be located in the `binary` directory.