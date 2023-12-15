# VulcanDB：一个基于属性图模型的BIM数据库

## 简介

VulcanDB是一个BIM数据库，支持BIM数据的存储、查询、分析等功能。VulcanDB的数据模型是基于属性图模型的，将BIM数据抽象为节点和边，节点和边上的属性可以用来描述BIM数据的特征。

* 提供基于PGQL的BIM查询语言，用户可以以图匹配的形式查询BIM模型中的任意数据。
* 提供BIM构件几何分析和几何计算函数。
* 基于PGQL声明式语言可以进行二/三维模型编辑。
* BIM数据存储，支持导入导出及格式转换。
* 多模数据集成支持，可将倾斜摄影、点云等数据与BIM构件关联。

## 安装

### Linux

安装全局依赖

```BASH
sudo apt install zlib1g-dev boost-dev libboost-all-dev libxml2-dev google-perftools libspdlog-dev
```

初始化第三方依赖

```BASH
git clone --recurse-submodules
```