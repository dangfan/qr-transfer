qr-transfer
===========

## Compilation

Required Libraries:

* opencv 2.4.7 (You may install it in C:/opencv)
* zbar 0.10 (You may install it in the default path)

If you install these libraries in other folders, please set the correct building path in Visual Studio.

The dlls are provided in the folder `dlls`.

## Running

### Sender

The program that send a file.

    sender filename fps compressed

The three arguments are:

* filename: the filename that you want to send.
* fps: frame rate of sender. Set to 20 if you are not sure.
* compressed: 0 for no compression, 1 for compression.

### Agent

The program that receive a file and then send it.

    agent path fps compressed

* path: the path that you want to write the file.
* fps: frame rate of sender. Set to 20 if you are not sure.
* compressed: 0 for no compression, 1 for compression.

### Receiver

The program that receive a file.

    receiver path compressed

* path: the path that you want to write the file.
* compressed: 0 for no compression, 1 for compression.

## How to use

Running the programs in different laptops first. Then try to move the laptops so that you can see the FPS on both laptops greater than 10. After that, click left button of the receiver laptop. Finally, click right button of the sender laptop.