#!/bin/sh

mode=$1
filename=$2

# Simulates an editor writing four passwords.
simple() {
cat > $filename << EOF
# my passwords
strawberry red
raspberry red
blackberry black
grapefruit yellow
EOF
}

# Four other passwords for alternate tests.
alt() {
cat > $filename << EOF
tiger feline with stripes
cat black
dog white
rat grey
EOF
}

# Simulates an editor writing one password and taking 2 seconds to do so.
slow() {
cat > $filename << EOF
roses red
EOF
sleep 2
}

case $mode in
	alt)
		alt
		;;
	slow)
		slow
		;;
	*)
		simple
		;;
esac

exit 0
