#ifndef MU_MATRIX_H
#define MU_MATRIX_H

#include "MU_functions.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// this needs a lot of cleanup

// these probably shouldn't be typed, for our sanitiy, who needs an int matrix? not us
/* we need to decide on a standard though. I think we should use float
   but if we wan't more precision we would need to make another type, maybe double is best for this?*/

typedef double** MU_matrix;

typedef struct {
    u32 columns;
    u32 rows;
} MU_matrix_size;

MU_matrix_size MU_get_matrix_size(MU_matrix matrix){
    return (MU_matrix_size){*matrix[-2], *matrix[-1]};
}

MU_matrix MU_new_matrix(u32 columns, u32 rows){
    MU_matrix matrix = calloc(columns + 2, sizeof(double*));
    matrix += 2; // shift by 2xsizseof(double*)

    double* columns_p = malloc(sizeof(double));
    double* rows_p = malloc(sizeof(double));

    *columns_p = columns;
    *rows_p = rows;

    matrix[-2] = columns_p;
    matrix[-1] = rows_p;

    for (u32 x = 0; x < columns; x++){
        matrix[x] = calloc(rows, sizeof(double));
    }

    return matrix;
}

//tried to do this with pointers instead of using a vec, I still don't really know how they work
MU_vec2u MU_index_to_coords(u32 index, u32 max_x){
    return (MU_vec2u){index % max_x, index / max_x};
}

//and since I'm using a vec for the last one, I should use one here
u32 MU_coords_to_index(MU_vec2u coords, u32 max_x){
    return (u32){coords.x + coords.y*max_x};
}

void MU_print_matrix(MU_matrix matrix){
    MU_matrix_size size = MU_get_matrix_size(matrix);

    printf("(%dx%d)\n+", size.columns, size.rows);

    for (u32 j = 0; j < size.columns; j++){
        printf("----------+");
    }

    printf("\n");


    for (u32 j = 0; j < size.rows; j++){
        printf("|");

        for (u32 i = 0; i < size.columns; i++){
            printf("%10f|", matrix[i][j]);
        }

        printf("\n+");

        for (u32 _ = 0; _ < size.columns; _++){
            printf("----------+");
        }

        printf("\n");
    }
}

void MU_free_matrix(MU_matrix matrix){
    u32 columns = *matrix[-2];

    for (u32 x = 0; x < columns; x++){
        free(matrix[x]);
    }

    free(matrix - 2);    
}

MU_matrix MU_vec2d_to_matrix(MU_vec2d vec){
    MU_matrix matrix = MU_new_matrix(2, 1);

    matrix[0][0] = vec.x;
    matrix[1][0] = vec.y;

    return matrix;
}

MU_matrix MU_vec3d_to_matrix(MU_vec3d vec){
    MU_matrix matrix = MU_new_matrix(3, 1);

    matrix[0][0] = vec.x;
    matrix[1][0] = vec.y;
    matrix[2][0] = vec.z;

    return matrix;
}

MU_matrix MU_vec3d_to_matrix_4x1(MU_vec3d vec){
    MU_matrix matrix = MU_new_matrix(4, 1);

    matrix[0][0] = vec.x;
    matrix[1][0] = vec.y;
    matrix[2][0] = vec.z;
    matrix[3][0] = 1;

    return matrix;
}

MU_vec3d MU_matrix_4x1_to_vec3d(MU_matrix matrix){
    return (MU_vec3d){matrix[0][0], matrix[1][0], matrix[2][0]};
}

MU_matrix MU_matrix_transpose(MU_matrix matrix){
    MU_matrix_size size = MU_get_matrix_size(matrix);

    MU_matrix transposed_matrix = MU_new_matrix(size.rows, size.columns);

    for (u32 y = 0; y < size.rows; y++){
        for (u32 x = 0; x < size.columns; x++){
            transposed_matrix[y][x] = matrix[x][y];
        }
    }

    return transposed_matrix;
}

MU_matrix MU_matrix_add(MU_matrix a, MU_matrix b){
    MU_matrix_size size_a = MU_get_matrix_size(a);
    MU_matrix_size size_b = MU_get_matrix_size(b);
    
    if(size_a.columns != size_b.columns || size_a.rows != size_b.rows){
        return 0;
    }

    MU_matrix sum_matrix = MU_new_matrix(size_a.columns, size_a.rows);
    
    for (u32 y = 0; y < size_a.rows; y++){
        for (u32 x = 0; x < size_a.columns; x++){
            sum_matrix[x][y] = a[x][y] + b[x][y];
        }
    }

    return sum_matrix;
}

MU_matrix MU_matrix_sub(MU_matrix a, MU_matrix b){
    MU_matrix_size size_a = MU_get_matrix_size(a);
    MU_matrix_size size_b = MU_get_matrix_size(b);
    
    if(size_a.columns != size_b.columns || size_a.rows != size_b.rows){
        return 0;
    }

    MU_matrix diff_matrix = MU_new_matrix(size_a.columns, size_a.rows);
    
    for (u32 y = 0; y < size_a.rows; y++){
        for (u32 x = 0; x < size_a.columns; x++){
            diff_matrix[x][y] = a[x][y] - b[x][y];
        }
    }

    return diff_matrix;
}

MU_matrix MU_matrix_mul(MU_matrix a, MU_matrix b){
    MU_matrix_size size_a = MU_get_matrix_size(a);
    MU_matrix_size size_b = MU_get_matrix_size(b);
    
    //rows must equal columns
    if(size_a.rows != size_b.columns){
        return 0;
    }

    //defining columns first here cus it's easier to think in terms of x and y (columns are x and rows are y)
    //note: this really helped because everything was wrong before I did this, I'm really tired
    
    u32 product_columns = size_b.columns;
    u32 product_rows = size_a.rows;
    size_t product_length = product_rows * product_columns;
    MU_matrix product_matrix = MU_new_matrix(product_columns, product_rows);

    for(u32 i = 0; i < product_length; i++){
        //getting coords in the product matrix from the index
        MU_vec2u current_coords = MU_index_to_coords(i, product_columns);
        u32 current_collum = current_coords.x;
        u32 current_row = current_coords.y;
        u32 depth = size_a.columns; 

        double sum = 0;
        for(u32 p = 0; p < depth; p++){
            sum += a[p][current_row] * b[current_collum][p];
        }

        product_matrix[current_collum][current_row] = sum;
    }

    return product_matrix;
}

// FIX THIS!! needs to be updated to double-indexed matricies
MU_matrix MU_matrix_mul_assign(MU_matrix a, MU_matrix b){
    MU_matrix_size size_a = MU_get_matrix_size(a);
    MU_matrix_size size_b = MU_get_matrix_size(b);
    
    //rows must equal columns
    if(size_a.rows != size_b.columns){
        return 0;
    }

    //defining columns first here cus it's easier to think in terms of x and y (columns are x and rows are y)
    //note: this really helped because everything was wrong before I did this, I'm really tired
    
    u32 product_columns = size_b.columns;
    u32 product_rows = size_a.rows;
    size_t product_length = product_rows * product_columns;
    MU_matrix product_matrix = MU_new_matrix(product_columns, product_rows);

    for(u32 i = 0; i < product_length; i++){
        //getting coords in the product matrix from the index
        MU_vec2u current_coords = MU_index_to_coords(i, product_columns);
        u32 current_collum = current_coords.x;
        u32 current_row = current_coords.y;
        u32 depth = size_a.columns; 

        double sum = 0;
        for(u32 p = 0; p < depth; p++){
            sum += a[p][current_row] * b[current_collum][p];
        }

        product_matrix[current_collum][current_row] = sum;
    }

    memcpy(a, product_matrix, sizeof(double) * product_length);

    MU_free_matrix(product_matrix);

    return a;
}

MU_matrix MU_4x4_x_4x1(MU_matrix a, MU_matrix b){
    MU_matrix c = MU_new_matrix(4, 1);

    c[0][0] = a[0][0] * b[0][0] + a[0][0] * b[1][0] + a[0][0] * b[2][0] + b[3][0]; 
    c[1][0] = a[1][0] * b[0][1] + a[1][0] * b[1][1] + a[1][0] * b[2][1] + b[3][1]; 
    c[2][0] = a[2][0] * b[0][2] + a[2][0] * b[1][2] + a[2][0] * b[2][2] + b[3][2]; 
    c[3][0] = a[3][0] * b[0][3] + a[3][0] * b[1][3] + a[3][0] * b[2][3] + b[3][3]; 

    return c;
}

MU_matrix MU_4x4_x_4x4(MU_matrix b, MU_matrix a) {
    MU_matrix c = MU_new_matrix(4, 4);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++)
                c[j][i] += a[i][k] * b[k][j];
    }

    return c;
}

void MU_4x4_x_4x4_to_c(MU_matrix a, MU_matrix b, MU_matrix c) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            c[j][i] = 0;

            for (int k = 0; k < 4; k++)
                c[j][i] += a[i][k] * b[k][j];
    }
}

void MU_4x4_clear_matrix(MU_matrix matrix){
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            matrix[j][j] = 0;
}

#endif