## 1. 安装依赖

```
sudo apt install zlib1g-dev
```

```
git clone https://github.com/jbeder/yaml-cpp.git
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make install

或

git clone --recurse-submodules
```

## 2. 配置实验环境
在config.yaml中配置实验环境，包括实验数据集路径、实验结果保存路径、实验参数等。