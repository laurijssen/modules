cmd_/home/kali/projects/modules/debugfs_driver/Module.symvers :=  sed 's/ko$$/o/'  /home/kali/projects/modules/debugfs_driver/modules.order | scripts/mod/modpost -m     -o /home/kali/projects/modules/debugfs_driver/Module.symvers -e -i Module.symvers -T - 
