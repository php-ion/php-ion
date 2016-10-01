
- `docker build --force-rm --no-cache --tag=ion-dbg:stage1 stage1`
- `docker build --force-rm --no-cache --tag=ion-dbg:stage2 stage2`
- `docker build --force-rm --no-cache --tag=ion-dbg:stage3 stage3`
- `docker run --cap-add=SYS_PTRACE --security-opt=apparmor:unconfined --security-opt=seccomp:unconfined --rm -v /Volumes/Data/Dev/php-ion:/data/php-ion -p 8017:8018 --name=ion-gdbserver ion-dbg:stage3`
- `docker exec -it ion-dbgserver /data/ion-orig/bin/ionizer.php --docker-sync-to=/data/ion-dev --make --dev --gdb-server=127.0.0.1:8017`