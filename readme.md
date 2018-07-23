### Testing

This article is about the connection testing of intelliPWR project. On the implementation steps, you must individually check the all step that given at below for avoiding the un-solicited status If you can pass all these steps, that's mean your device can work as very well on this project.

Otherwise, it will make a BOOM!

#### How to test an Slave and Master Device?

It is so easy. The table is below. Check them all.

|                                                    | Device X1          | Device X2 | Device X3          | Description |
| -------------------------------------------------- | :----------------: | :-------: | :----------------: | :---------: |
| ID                                                 | :heavy_check_mark: | :x:       | :heavy_check_mark: | - |
| I<sup><sup>2</sup></sup>C<sub><sub>GND</sub></sub> | :heavy_check_mark: | :x:       | :heavy_check_mark: | - |
| I<sup><sup>2</sup></sup>C<sub><sub>SCL</sub></sub> | :heavy_check_mark: | :x:       | :heavy_check_mark: | D2 |
| I<sup><sup>2</sup></sup>C<sub><sub>SDA</sub></sub> | :heavy_check_mark: | :x:       | :x:                | D1 |
| 220V<sub><sub>VCC</sub></sub>                      | :heavy_check_mark: | :x:       | :x:                | - |
| 220V<sub><sub>GND</sub></sub>                      | :heavy_check_mark: | :x:       | :heavy_check_mark: | - |
| SSR                                                | :heavy_check_mark: | :x:       | :heavy_check_mark: | D5 |
| LED<sub><sub>R</sub></sub>                         | :x:                | :x:       | :x:                | D6 |
| LED<sub><sub>GB</sub></sub>                        | :x:                | :x:       | :x:                | D7 |

And also the socket pin orders are given at below. This given orders must be same with your slave device. Otherwise, SCL will comm with SDA and SDA will comm with SCL.

|                    | Pin 1              | Pin 2              | Pin 3              | Pin 4              | Pin 5              |
| ------------------ | :----------------: | :----------------: | :----------------: | :----------------: | :----------------: |
| Pin                | D1                 | D1                 | GND                | D2                 | D2                 |
| Status             | SDA                | SDA                | GND                | SCL                | SCL                |
