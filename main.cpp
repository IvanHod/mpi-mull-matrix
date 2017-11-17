#include <iostream>
#include <mpi.h>
#include <vector>
#include <fstream>

#define SIZE 100

typedef std::vector<std::vector<int>> Matrix;

void writeFile(Matrix matrix);

void fullMatrixs(Matrix &m, int);

void pathMulMatrix(const Matrix &m1, const Matrix &m2, Matrix &m, int, int);

/**
 * Число строк должно быть кратно числу потоков
 * */

int main() {
    Matrix m1, m2, m;
    fullMatrixs(m1, 1);
    fullMatrixs(m2, 1);
    fullMatrixs(m, 0);
    double time = 0;

    MPI::Init();

    int rank = MPI::COMM_WORLD.Get_rank(),
            size = MPI::COMM_WORLD.Get_size();
    int offset = SIZE / size;
    MPI::Status status;

    if (rank == 0) {
        time = MPI::Wtime();

        for (int i = 1; i < size; ++i) {
            int newLine = offset * i;
            MPI::COMM_WORLD.Send(&newLine, 1, MPI::INT, i, 0);
        }
        pathMulMatrix(m1, m2, m, 0, 0 + offset);

        for (int i = 1; i < size; ++i) {
            int otherStrInit;
            MPI::COMM_WORLD.Recv(&otherStrInit, 1, MPI::INT, i, 0, status);
            for (int j = otherStrInit; j < otherStrInit + offset; ++j) {
                MPI::COMM_WORLD.Recv(&m[j].front(), SIZE, MPI::INT, i, j + 1, status);
            }
        }
        time = MPI::Wtime() - time;
        std::cout << "Result time: " << time << std::endl;
        writeFile(m);
    } else {
        int startRow;
        MPI::COMM_WORLD.Recv(&startRow, 1, MPI::INT, 0, MPI::ANY_TAG, status);

        int endRow = startRow + offset;
        pathMulMatrix(m1, m2, m, startRow, endRow);

        MPI::COMM_WORLD.Send(&startRow, 1, MPI::INT, 0, 0);
        for (int i = startRow; i < endRow; ++i) {
            MPI::COMM_WORLD.Send(&m[i].front(), SIZE, MPI::INT, 0, i + 1);
        }
    }
    MPI::Finalize();
    return 0;
}

void writeFile(Matrix matrix) {
    std::string fileName("test/matrix_" + std::to_string(SIZE) + ".txt");
    std::ofstream outputFile;
    outputFile.open(fileName);
    outputFile << SIZE << " " << SIZE << std::endl;
    for(auto row : matrix) {
        for(const auto &val : row)
            outputFile << val << " ";
        outputFile << std::endl;
    }
    outputFile.close();
}

void fullMatrixs(Matrix &m, int value) {
    for(int i = 0; i < SIZE; i++) {
        std::vector<int> line;
        m.push_back(line);
        for(int j = 0; j < SIZE; j++)
            m[i].push_back(value);
    }
}

void pathMulMatrix(const Matrix &m1, const Matrix &m2, Matrix &m, int startRow, int endRow) {
    for (int i = startRow; i < endRow; i++) {
        for(int j = 0; j < SIZE; j ++) {
            for(int k = 0; k < SIZE; k ++) {
                m[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
}