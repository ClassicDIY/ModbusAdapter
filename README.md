
<h1>ModbusAdapter</h1>
<h2>Adds Modbus TCP over WIFI to a Modbus RTU Serial device</h2>
<p>
  
|If you find this project useful or interesting, please help support further development!|[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=graham.a.ross%40gmail.com&item_name=Support+ModbusAdapter+development&currency_code=USD&source=url)|
|---|---|

[![Hits](https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2FClassicDIY%2FModbusAdapter&count_bg=%2379C83D&title_bg=%23555555&icon=&icon_color=%23E7E7E7&title=hits&edge_flat=false)](https://hits.seeyoufarm.com)

[![GitHub All Releases](https://img.shields.io/github/downloads/ClassicDIY/ModbusAdapter/total.svg?style=for-the-badge)](https://github.com/ClassicDIY/ModbusAdapter/releases)
[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/ClassicDIY/ModbusAdapter.svg?style=for-the-badge)](https://github.com/ClassicDIY/ModbusAdapter/releases)
[![GitHub issues](https://img.shields.io/github/issues/ClassicDIY/ModbusAdapter?style=for-the-badge)](https://github.com/ClassicDIY/ModbusAdapter/issues)

<p>
Please refer to the <a href="https://github.com/ClassicDIY/ModbusAdapter/wiki">ModbusAdapter wiki</a> for more information.
</p>

<h2>Parts</h2>

|<a href="https://www.aliexpress.com/item/32826540261.html?src=google&src=google&albch=shopping&acnt=494-037-6276&isdl=y&slnk=&plac=&mtctp=&albbt=Google_7_shopping&aff_platform=google&aff_short_key=UneMJZVf&&albagn=888888&albcp=7386552844&albag=80241711349&trgt=743612850714&crea=en32826540261&netw=u&device=c&albpg=743612850714&albpd=en32826540261&gclid=Cj0KCQjw-r71BRDuARIsAB7i_QMqV6A_E4zdDcSiXs2j3qIUm4cIgdCFfkDs1Egmak4QgCXrvfcQXAkaAu2WEALw_wcB&gclsrc=aw.ds"> ESP32 Dev Module</a>|<img src="./Pictures/esp32.jpg" width="120"/>|
|---|---|
|<a href="https://www.canakit.com/rs232-interface-module-ck1007-uk1007.html"> RS232 Interface Module </a>|<img src="./Pictures/RS232.jpg" width="120"/>|

<h2>Wiring diagram</h2>

RS232 Interface Module | ESP32 |
--- | --- |
VCC | 3.3V |
RX | GPIO16 (RX2) |
TX | GPIO17 (TX2)|
Gnd | Gnd |

<p align="center">
  <img src="./Pictures/Wiring.png" width="800"/>
</p>

<h2>RJ11 to male DE-9 connector Cable</h2>

<p align="center">
  <img src="./Pictures/CableClassic.png" width="800"/>
</p>

<h2>ModbusAdapter home page</h2>

<p align="center">
  <img src="./Pictures/home.png" width="320"/>
</p>

<h2>ModbusAdapter configure page</h2>
 *** Use admin/ModbusAdapter to access setup
 
<p align="center">
  <img src="./Pictures/Setup.png" width="320"/>
</p>

 ### Use the ClassicMonitor android app to view your data
 
 *** Connect to the ModbusAdapter IP Address
 
<a href='https://play.google.com/store/apps/details?id=ca.farrelltonsolar.classic&hl=en&pcampaignid=pcampaignidMKT-Other-global-all-co-prtnr-py-PartBadge-Mar2515-1'><img alt='Get it on Google Play' src='https://play.google.com/intl/en_us/badges/static/images/badges/en_badge_web_generic.png' width=200/></a>

<img src="http://graham22.github.io/Classic/classicmonitor/images_en/StateOfCharge_landscape.png" width="600"/>
