#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

typedef struct Matrix {
    int rows, cols;
    double** values;
} Matrix;

Matrix * newMatrix(int rows, int cols) {
    Matrix* m = malloc(sizeof(Matrix));
    m->rows = rows;
    m->cols = cols;
    m->values =  (double**)malloc(rows * sizeof(double*));
    for (int r = 0; r < rows; r++) {
        m->values[r] = (double*)malloc(cols * sizeof(double));
    }
    return m;
}

Matrix* dotProduct(Matrix* a, Matrix* b) {
    Matrix* m = newMatrix(a->rows, b->cols);
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            m->values[i][j] = 0;
            for (int k = 0; k < a->cols; k++) {
                m->values[i][j] += a->values[i][k] * b->values[k][j];
            }            
        }
    }
    return m;
}

Matrix* calculateLayer(Matrix* a, Matrix* b, double (* activation)(double)) {
    Matrix* m = newMatrix(a->rows, b->cols);
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            m->values[i][j] = 0;
            for (int k = 0; k < a->cols; k++) {
                m->values[i][j] += a->values[i][k] * b->values[k][j];
            }
            if (activation != NULL) {
                m->values[i][j] = activation(m->values[i][j]);
            }
        }
    }
    return m;
}

double randomWeight() {
    return (double)rand() / RAND_MAX;
}

void randomize(Matrix *m) {
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            m->values[i][j] = randomWeight();
        }
    }
}

void printMatrix(Matrix* m) {
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            printf("\t%f", m->values[i][j]);
        }
        printf("\n");
    }
}



typedef struct Layer {
    Matrix* weights;
} Layer;

typedef struct NeuralNetwork {
    Layer* layers;
} NeuralNetwork;


int main(int argc, char** argv) {
  /*  if (argc < 2) {
        printf("Usage:\n");
        printf("\tnn [# inputs] [# neurons layer 1] [# neurons layer 2] ... [# neurons layer n]\n");
        printf("\nExamples:\n");
        printf("\nE1. A network with 4 inputs, and a single layer with a single neuron:\n");
        printf("\t./nn 4 1\n");
        printf("\nE2. A network with 10 inputs, layer 1 with 10 neurons, layer 2 with 5 neurons:\n");
        printf("\tnn 4 10 5\n");
        exit(1);
    }
    */

    double or_dataset[4][3] = {
        {0.0f,0.0f,0.0f},
        {0.0f,1.0f,1.0f},
        {1.0f,0.0f,1.0f},
        {1.0f,1.0f,1.0f}
    };
    double and_dataset[4][3] = {
        {0.0f,0.0f,0.0f},
        {0.0f,1.0f,0.0f},
        {1.0f,0.0f,0.0f},
        {1.0f,1.0f,1.0f}
    };

    Matrix* input = newMatrix(2, 1);
    input->values[0][0] = 0;
    input->values[0][0] = 0;

    Matrix* l1 = newMatrix(3, 2);
    randomize(l1);
    
    //Matrix* c = calculateLayer(a,b, sigmoid);
    printMatrix(l1);
}