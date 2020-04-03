# Cloth-Toy
A cloth physics application which allows user interaction

This application started from an assignment in CSE 169, however a few adjustments have been made. The cloth is simulated using a grid of particles linked together by spring dampers. Forces such as gravity and drag are calculated using their corresponding physics equations and particle positions are updated using the forward euler method.

The application allows the user to move the cloth around the horizontal plane using the W,A,S,D keys, and vertically using the U,I keys. Wind can be controlled by using the arrow keys to increase or decrease wind speed - it can be turned on or off using the B key.

As an extension to the assignment, there is extra functionality that allows the user to pick points on the cloth using their mouse and to be able to move these points around.