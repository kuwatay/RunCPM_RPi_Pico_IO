# RunCPM_RPo_Pico + IO port emulation

This program is a fork of RunCPM_RPo_Pico

 ![RunCPM_RPo_Pico](https://github.com/guidol70/RunCPM_RPi_Pico)

This repository provides a program that extends I/O port emulation for RunCPMRPoPico. By using the extended functionality provided in this repository, you can directly control GPIOs through Z80 I/O instructions, making it easy to perform external control operations.

![pico_io_01] (https://github.com/kuwatay/RunCPM_RPi_Pico_IO/images/pico_io_01.jpg)

For parts outside the extension, please refer to the original repository.

Specifically, two 8-bit ports have been added, which can be directly accessed by the Z80 CPU using I/O instructions.

## Address Mapping
Ports are mapped Z80 IO address as follows;

```
PORT A
	I/O register: $4
	Ddata Direction Register : $5
	Pull-Up Register: $6
	
PORT B
	I/O register: $0
	Ddata Direction Register : $1
	Pull-Up Register: $2
```
Each pin (bit) of the port can be individually configured as either an input or an output.　The default configuration of port pins is set to input.

To set the port pin as an output, it is necessary to write a 1 to the corresponding bit in the DDR (Data Direction Register).To set it as an input, you need to write a 0 to the corresponding bit in the DDR. 

When a pin is configured as an output, the value set in the I/O register is output to the corresponding GPIO.
When the pin is configured as an input, the value of the corresponding GPIO can be read.

Similarly, to enable the pull-up on a port pin, you need to write a 1 to the corresponding bit in the PUR (Pull-Up Register).The Pull-Up Register is only effective when the pin is configured as an input.

The DDR and PUR can also be read to retrieve the values that have been set

## GPIO Mappinng
The port corresponds to the physical GPIOs as follows.

```
PORT A
	A0-A7: GPIO2-GPIO9
	
PORT B
	B0-B5: GPIO10-GPIO15
	B6-B7: GPIO20-GPIO21
	
```

By modifying the definitions in the source code, you can assign available GPIOs to achieve a different pin configuration.

## Example
Using the MS Basic included with RunCPM at the A drive, you can easily control I/O ports. 

The following program sets PORT A as output and sequentially outputs values from 0 to 255.

```
10 OUT 5,255
20 FOR I=0 TO 255
30  OUT 4,I
40  FOR J=0 TO 1000
50  NEXT J
60 NEXT I
```
