The control box i made for my reverse osmosis filter. I doubt anyone needs this.

![img-1](./img/photo_2024-09-13_20-38-01.jpg?raw=true)

![img-2](./img/photo_2024-09-13_20-38-23.jpg?raw=true)

![img-3](./img/photo_2024-09-13_20-38-27.jpg?raw=true)

## Main display
Off -> Blank\
Idle -> filter total time, days\
Work -> filtering time, minutes\
Flush -> Spin\
No water -> "drY"\
Timeout -> "Err"\
Short press: Off->on, Flush/Nowater/Timeout->idle, Other->lamp on/off\
Long press: menu\

### Menu
OFF -> Turn off\
FLU -> Start manual flush\
C00 -> Filter total time, hrs/thousands\
C01 -> Filter work time, hrs/thousands\
C02 -> Total on time, hrs/thousands\
C03 -> Total run time, hrs/thousands\
C04 -> Total start count, pcs/thousands\
C05 -> Total flush count, pcs/thousands\
P00 -> Manual flushing time 0..60/1 min\
P01 -> Auto flushing time 0..900/10 sec\
P02 -> Work time flush threshold 0..990/10 min\
P03 -> Total time flush threshold 0..240/5 hr\
P04 -> Timeout 0..360/5 min\
P05 -> Nowater delay 0..60/1 sec\
P06 -> Extra work time after HP switch off 0..360/5 sec\
CLr -> Reset filter on time/work time (confirm yes/no)\
X.XX -> AIN_C voltage (ok=back)\
XX.X -> Input voltage (ok=back)\
Long press: edit parameter/ok\
Short press: next parameter\

### Parameter edit / confirm menu\
short press: increase value / toggle confirm\
long press: save\

### Parameter display\
short press: back to menu\
long press: back to main display\

## Board pinout
DIA -> low pressure switch\
DIB -> high pressure switch\
DQA -> pump + inlet valve\
DQB -> flush valve\
DQC -> lamp\

## ATmega328 fuses
int.RC 8MHz 0ms+6CK DIV8\
BOR@1.8V
