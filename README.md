# N-Body simulation with OpenGL

## N-Body problem

The N-Body problem is a physical problem that consists of a system of N bodies interacting with each other gravitationally. The goal is to compute the evolution of the system in time. To do this, we use the Newtonian equations of motion and the gravitational force between two bodies.

For more information, see [Wikipedia](https://en.wikipedia.org/wiki/N-body_problem).

## Language and paradigm

To achiving this goal, we use the C++ with the parallel programming parading MPI. The MPI is a message passing interface that allows the communication between processes. The MPI is a standard that is implemented in many languages and libraries, such as C, C++, Fortran etc...
In this case, we used the implementation of the MPI in C++ by the [OpenMPI](https://www.open-mpi.org/) library.

## How to run

To run the program, you need to have the OpenMPI library installed. To install it, you can follow the instructions in the [OpenMPI website](https://www.open-mpi.org/faq/?category=building#easy-build).

Also, you need to have the OpenGL library installed. To install it, you can follow the instructions in the [OpenGL website](https://www.opengl.org/wiki/Getting_Started).

Opengl implementation in C++ is done by the [GLFW](https://www.glfw.org/) library. To install it, you can follow the instructions in the [GLFW website](https://www.glfw.org/docs/latest/compile_guide.html).

And for the last, you need glad to load the OpenGL functions. To install it, you can follow the instructions in the [Glad website](https://glad.dav1d.de/).

After installing all the libraries, you can compile the program thanks to the Makefile. To do this, you need to run the following command in the terminal:

```bash
make [SYMMETRY=true] [VISUALISATION=true]
```

The SYMMETRY parameter is optional. If you omit it, the program will run without symmetry forces calculation. If you want to run the program with symmetry, you need to add the parameter SYMMETRY. For example:

```bash
make SYMMETRY=true
```

If you want visualize the simulation, you need to run the following command in the terminal:

```bash
make SYMMETRY=true VISUALISATION=true
```

## How to use

After compiling the program, you can run it with the following command:

```bash
mpirun -np [NUMBER_OF_PROCESSES] ./bin/N-Body
```

## Contributors

The project id done in the context of the course "Distributed Systems" at the University of Dijon. <br>
The contributors are:

- [Maxime DUPONT Github](https://github.com/maxime-dupont01)
- [Christian Tomasino Github](https://github.com/ChrisTom-94)
