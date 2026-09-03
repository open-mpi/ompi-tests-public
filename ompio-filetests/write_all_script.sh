#! /bin/bash


echo "fcoll=vulcan num_aggregators=1"
mpirun --mca io ompio --mca fcoll vulcan --mca io_ompio_num_aggregators 1  -np 6 ./write_all

echo "fcoll=vulcan num_aggregators=2"
mpirun --mca io ompio --mca fcoll vulcan --mca io_ompio_num_aggregators 2  -np 6 ./write_all

echo "fcoll=vulcan num_aggregators=3"
mpirun --mca io ompio --mca fcoll vulcan --mca io_ompio_num_aggregators 3  -np 6 ./write_all

echo "fcoll=vulcan num_aggregators=4"
mpirun --mca io ompio --mca fcoll vulcan --mca io_ompio_num_aggregators 4  -np 6 ./write_all

echo "fcoll=vulcan num_aggregators=5"
mpirun --mca io ompio --mca fcoll vulcan --mca io_ompio_num_aggregators 5  -np 6 ./write_all

echo "fcoll=vulcan num_aggregators=6"
mpirun --mca io ompio --mca fcoll vulcan --mca io_ompio_num_aggregators 6  -np 6 ./write_all

echo "fcoll=dynamic num_aggregators=1"
mpirun --mca io ompio --mca fcoll dynamic --mca io_ompio_num_aggregators 1  -np 6 ./write_all

echo "fcoll=dynamic num_aggregators=2"
mpirun --mca io ompio --mca fcoll dynamic --mca io_ompio_num_aggregators 2  -np 6 ./write_all

echo "fcoll=dynamic num_aggregators=3"
mpirun --mca io ompio --mca fcoll dynamic --mca io_ompio_num_aggregators 3  -np 6 ./write_all

echo "fcoll=dynamic num_aggregators=4"
mpirun --mca io ompio --mca fcoll dynamic --mca io_ompio_num_aggregators 4  -np 6 ./write_all

echo "fcoll=dynamic num_aggregators=5"
mpirun --mca io ompio --mca fcoll dynamic --mca io_ompio_num_aggregators 5  -np 6 ./write_all

echo "fcoll=dynamic num_aggregators=6"
mpirun --mca io ompio --mca fcoll dynamic --mca io_ompio_num_aggregators 6  -np 6 ./write_all

echo "fcoll=dynamic_gen2 num_aggregators=1"
mpirun --mca io ompio --mca fcoll dynamic_gen2 --mca io_ompio_num_aggregators 1 -np 6 ./write_all

echo "fcoll=dynamic_gen2 num_aggregators=2"
mpirun --mca io ompio --mca fcoll dynamic_gen2 --mca io_ompio_num_aggregators 2 -np 6 ./write_all

echo "fcoll=dynamic_gen2 num_aggregators=3"
mpirun --mca io ompio --mca fcoll dynamic_gen2 --mca io_ompio_num_aggregators 3 -np 6 ./write_all

echo "fcoll=dynamic_gen2 num_aggregators=4"
mpirun --mca io ompio --mca fcoll dynamic_gen2 --mca io_ompio_num_aggregators 4 -np 6 ./write_all

echo "fcoll=dynamic_gen2 num_aggregators=5"
mpirun --mca io ompio --mca fcoll dynamic_gen2 --mca io_ompio_num_aggregators 5 -np 6 ./write_all

echo "fcoll=dynamic_gen2 num_aggregators=6"
mpirun --mca io ompio --mca fcoll dynamic_gen2 --mca io_ompio_num_aggregators 6 -np 6 ./write_all

echo "fcoll=individual"
mpirun --mca io ompio --mca fcoll individual  -np 6 ./write_all

echo "fcoll=two_phase num_aggregators=1"
mpirun --mca io ompio --mca fcoll two_phase --mca io_ompio_num_aggregators 1  -np 6 ./write_all

echo "fcoll=two_phase num_aggregators=2"
mpirun --mca io ompio --mca fcoll two_phase --mca io_ompio_num_aggregators 2  -np 6 ./write_all

echo "fcoll=two_phase num_aggregators=3"
mpirun --mca io ompio --mca fcoll two_phase --mca io_ompio_num_aggregators 3  -np 6 ./write_all

echo "fcoll=two_phase num_aggregators=4"
mpirun --mca io ompio --mca fcoll two_phase --mca io_ompio_num_aggregators 4  -np 6 ./write_all

echo "fcoll=two_phase num_aggregators=5"
mpirun --mca io ompio --mca fcoll two_phase --mca io_ompio_num_aggregators 5  -np 6 ./write_all

echo "fcoll=two_phase num_aggregators=6"
mpirun --mca io ompio --mca fcoll two_phase --mca io_ompio_num_aggregators 6  -np 6 ./write_all



exit 1
