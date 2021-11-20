if [ $# -eq 0 ]
then
	echo "Usage: ./install.sh <pathname> ";
	exit;
fi

if [ ! -d $1 ]
then
	echo "Error: Directory $1 DOES NOT exist.";
	while true; do
		read -p "Do you wish to create $1 directory? [Y/n] " yn
		case $yn in
			[Y]* ) mkdir $1; break;;
			[n]* ) exit;;
			* ) "Please, answer Y for yes or n for no.";;
		esac
	done
fi

echo "Begin program installation on $1 ... ";

unzip ./src.zip;
mv ./src $1;
cp ./help.sh $1;
cp run.sh $1;
cp ./README.txt $1;

echo "Program installed on $1";

dpkg --status libncurses-dev &> /dev/null
if [ $? -eq 0 ]
then
	echo "ncurses library already installed."
else
	echo "For compiling, ncurses library is needed.";
	while true; do
		read -p "Do you wish to continue? [Y/n] " yn
		case $yn in
			[Y]* ) sudo apt-get install -y libncurses-dev; break;;
			[n]* ) exit;;
			* ) "Please, answer Y for yes or n for no.";;
		esac
	done
fi

echo "Begin sources' compilation ...";

gcc $1/src/master/master.c -o $1/src/executables/master;
gcc $1/src/command/command.c -o $1/src/executables/command;
gcc $1/src/motor_x/motor_x.c -o $1/src/executables/motor_x;
gcc $1/src/motor_z/motor_z.c -o $1/src/executables/motor_z;
gcc $1/src/inspection/inspection.c -lncurses -lm -o $1/src/executables/inspection;
gcc $1/src/watchdog/wd.c -o $1/src/executables/wd;

echo "You can run the program in $1 with ./run.sh ";