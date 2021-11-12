gcc master.c -o master
gcc command.c -o command
gcc motor_x.c -o motor_x
gcc motor_z.c -o motor_z
gcc inspection.c -lncurses -lm -o inspection
gcc wd.c -o wd

./master