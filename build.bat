docker build . -t innoverse-patcher
docker run -it --rm -v .:/app innoverse-patcher
pause