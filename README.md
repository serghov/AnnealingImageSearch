# Motivation

This is just an experiment and I really dont think that can be used for anything useful but learning.

In order to better understand simulated annealing I decided to write a simple program that 
would find image inside another image and will be invariant to rotation, scale, and translation using the upper mentioned method.

So essentially the program choses two points, moves the test image so that the points would be its upper left and
upper right corners and calculates mean square difference between the modified test image and original image. 
After doing this a bunch of times it takes the points with minimal error (mean square difference) as answer. 
Chosing the points is done using simulated annealing.

Essentially we are choosing 4 variables (two points each with two coordinates) and minimizing a discrete function.

# Running

The project is straight from Netbeans but I think that you can just build it using the Makefile.

## Dependencies
* opencv 2.4.* (other versions will also probably work)
* c++11 supporting compiler, most modern ones do

## Results

Ones you have run the code you will get one of these

![Imgur](http://i.imgur.com/N85p50s.png)


The right one is the good result and the left one (or something like that) is the bad result.
In my experiment you get good reults half the time and bed results the other half. 
You can also see the current value of error and alpha in the command line.

