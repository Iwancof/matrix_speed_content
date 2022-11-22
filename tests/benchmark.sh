num=100

for i in $(seq 1 $num)
do
  echo $i
  ./host_main_normal | tee -a fuse_host_normal
  ./host_main_hard | tee -a fuse_host_hard
  ./docker_main_normal | tee -a fuse_doc_normal
  ./docker_main_hard | tee -a fuse_doc_hard
  python ./python_part.py | tee -a fuse_py
done
