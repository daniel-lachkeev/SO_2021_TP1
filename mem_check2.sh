#!/bin/sh
valgrind --leak-check=full --show-reachable=yes --track-origins=yes ./pcu prob03.txt 10 60
