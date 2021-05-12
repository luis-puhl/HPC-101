#!/bin/bash
#SBATCH -J pi-int                   # Job name
#SBATCH -p fast                     # Job partition
#SBATCH -n 1                        # Number of processes
#SBATCH -t 00:15:00                 # Run time (hh:mm:ss)
#SBATCH --cpus-per-task=40          # Number of CPUs per process
#SBATCH --output=%x.%j.out          # Name of stdout output file - %j expands to jobId and %x to jobName
#SBATCH --error=%x.%j.err           # Name of stderr output file
#SBATCH --mail-user=luispuhl@gmail.com
#SBATCH --mail-type=ALL
#SBATCH --account=u706298

# OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK} make -e run.txt
if ! srun --version &> /dev/null
then
    echo "Using sudo"
    SRUN="sudo"
else
    echo "Using srun"
    SRUN="srun"
fi
export NUM_STEPS=1000000000

echo "SLURM_CPUS_PER_TASK=${SLURM_CPUS_PER_TASK}" | tee job.log
echo "*** SEQUENTIAL ***" | tee -a job.log
${SRUN} singularity run pi-int.sif pi_seq ${NUM_STEPS} \
    | sed "s/^/SEQ\tUNITS=01\t/" | tee -a job.log
echo "*** PTHREAD ***" | tee -a job.log
for UNITS in {01,02,05,10,20,40}; do
    ${SRUN} singularity run pi-int.sif pi_pth ${NUM_STEPS} "${UNITS}" \
        | sed "s/^/PTH\tUNITS=${UNITS}\t/" | tee -a job.log
done
echo "*** OPENMP ***" | tee -a job.log
for UNITS in {01,02,05,10,20,40}; do
    export OMP_NUM_THREADS=${UNITS}
    ${SRUN} singularity run pi-int.sif pi_omp ${NUM_STEPS} \
        | sed "s/^/OMP\tUNITS=${UNITS}\t/" | tee -a job.log
done
echo "Done" | tee -a job.log
