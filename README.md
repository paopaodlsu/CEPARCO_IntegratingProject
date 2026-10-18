# CEPARCO Integrating Project milestone # 1 (proposal)
**Group Members**
- Hernaez, Raeka Estrelle
- Liwanag, Ram Miguel
- Serrano, Paul Rhazzel
- Tiu, Kyle 
---
# Project Title:
## SIMD-BASED AVX2 MATRIX MULTIPLICATION 

# Abstract 
Matrix multiplication is a mathematical operation used in various computing applications. This project aims to develop a program that performs matrix multiplication using AVX2 assembly to demonstrate how multiple data values can be processed at the same time. The program will generate two matrices with compatible dimensions and calculate their product using two approaches, a sequential implementation that processes the calculations one at a time and a parallel implementation using AVX2 assembly, which allows multiple values to be processed in a single instruction. The results of both implementations will be checked for correctness, and their execution times will be measured and compared across different matrix sizes. Through this comparison, the project aims to determine how using SIMD affects the execution time of matrix multiplication as the input size increases.

# Proposed Project Description 
## a. What are the inputs to your project?
The program will take in two matrices, A and B, that are populated with single-precision floating-point integers. The user will have to input four different dimension inputs: X rows and Y columns for matrix A, and Y rows and Z columns for matrix B. As it is imperative that the number of columns for matrix A and the number of rows for matrix B are equal, error checking will be implemented to ensure that this is enforced. These values will be used to construct the matrixes, which will then be filled with randomly generated single-precision floating-point integers. 
