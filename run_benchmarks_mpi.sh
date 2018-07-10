#!/usr/bin/env bash
N=10000000

srun -p q_student -N 1 --ntasks-per-node=1 ./mpiquick.bin > $HOME/bench2/n_${N}_N1n1
for i in {1..24}; do
    srun -p q_student -N 1 --ntasks-per-node=1 ./mpiquick.bin >> $HOME/bench2/n_${N}_N1n1;
done

srun -p q_student -N 1 --ntasks-per-node=2 ./mpiquick.bin > $HOME/bench2/n_${N}_N1n2
for i in {1..24}; do
    srun -p q_student -N 1 --ntasks-per-node=2 ./mpiquick.bin >> $HOME/bench2/n_${N}_N1n2;
done

srun -p q_student -N 1 --ntasks-per-node=4 ./mpiquick.bin > $HOME/bench2/n_${N}_N1n4
for i in {1..24}; do
    srun -p q_student -N 1 --ntasks-per-node=4 ./mpiquick.bin >> $HOME/bench2/n_${N}_N1n4;
done

srun -p q_student -N 1 --ntasks-per-node=8 ./mpiquick.bin > $HOME/bench2/n_${N}_N1n8
for i in {1..24}; do
    srun -p q_student -N 1 --ntasks-per-node=8 ./mpiquick.bin >> $HOME/bench2/n_${N}_N1n8
done

srun -p q_student -N 1 --ntasks-per-node=16 ./mpiquick.bin > $HOME/bench2/n_${N}_N1n16
for i in {1..24}; do
srun -p q_student -N 1 --ntasks-per-node=16 ./mpiquick.bin >> $HOME/bench2/n_${N}_N1n16;
done

srun -p q_student -N 2 --ntasks-per-node=16 ./mpiquick.bin > $HOME/bench2/n_${N}_N2n16
for i in {1..24}; do
srun -p q_student -N 2 --ntasks-per-node=16 ./mpiquick.bin >> $HOME/bench2/n_${N}_N2n16;
done

srun -p q_student -N 4 --ntasks-per-node=16 ./mpiquick.bin > $HOME/bench2/n_${N}_N4n16
for i in {1..24}; do
srun -p q_student -N 4 --ntasks-per-node=16 ./mpiquick.bin >> $HOME/bench2/n_${N}_N4n16;
done

srun -p q_student -N 8 --ntasks-per-node=16 ./mpiquick.bin > $HOME/bench2/n_${N}_N8n16
for i in {1..24}; do
srun -p q_student -N 8 --ntasks-per-node=16 ./mpiquick.bin >> $HOME/bench2/n_${N}_N8n16;
done

srun -p q_student -N 16 --ntasks-per-node=16 ./mpiquick.bin > $HOME/bench2/n_${N}_N16n16
for i in {1..24}; do
srun -p q_student -N 16 --ntasks-per-node=16 ./mpiquick.bin >> $HOME/bench2/n_${N}_N16n16;
done

srun -p q_student -N 32 --ntasks-per-node=16 ./mpiquick.bin > $HOME/bench2/n_${N}_N32n16
for i in {1..24}; do
srun -p q_student -N 32 --ntasks-per-node=16 ./mpiquick.bin >> $HOME/bench2/n_${N}_N32n16;
done
