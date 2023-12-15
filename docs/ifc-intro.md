# IFC标准详解

## IFC标准 - What is IFC?

### 1. EXPRESS建模语言

IFC是由BuildingSmart标准组织提出的建筑信息模型交换国际标准，使用EXPRESS建模语言描述。

EXPRESS是一种基于实体-关系模型的声明式建模语言，它用来描述一定领域的类，与这些类有关的信息或属性(如颜色、尺寸、形状等) 和这些类的约束 (如惟一性)。它也用来定义类之间的关系和这些关系上的约束。

![EXPRESS 示例](https://img.ironmanzzm.top/blog-img/20231215152821.png)

* 在类的定义中，所有表示它特征的属性和行为关系都要声明类的声明用关键词`ENTITY`开始和`END_ENTITY`结束。
* `SUPERTYPE OF` 表示一个类是另一个类的超类，即一个类是另一个类的子类。
* `SUBTYPE OF` 表示一个类是另一个类的子类。
* 属性可以用简单的数据类型(如实数、整数、字符串)来表达或者用其他的类，每种属性都和类有关系。
* 一个类可以有多个属性，每个属性都有一个名字和一个类型。
* `INVERSE` 表示一个属性是另一个属性的逆属性，即一个属性是另一个属性的反向关系。
* `UNIQUE` 表示一个类的属性的值是唯一的。
* `WHERE` 表示一个类的属性的值必须满足一定的条件。

#### 数据类型

EXPRESS 中的基本数据类型，包括 `INTEGER` 、`REAL` 、`STRING` 、`BOOLEAN` 、`LOGICAL` 、`NUMBER` 和 `BINARY` 。

![基本类型](https://img.ironmanzzm.top/blog-img/20231215163907.png)

聚合类型有四种: `ARRAY` 、`BAG` 、`LIST` 和 `SET`。其中`LIST` 和 `SET` 是应用最广的聚合数据类型。

![聚合类型](https://img.ironmanzzm.top/blog-img/20231215163947.png)

`Enumeration` 规定属性只能包含预定义字符串中的特定值。例如，

```EXPRESS
TYPE IfcActionRequestTypeEnum = ENUMERATION OF (
EMAIL,
FAX,
PHONE,
POST,
VERBAL,
USERDEFINED,
NOTDEFINED);
END_TYPE;
```

`SELECT` 类型的值可以是其定义中列出的任何类型的值。例如，IFC 数据模型中的 IfcObjectPlacement 类型是一个选择类型，它可以是 IfcLocalPlacement 或 IfcGridPlacement 类型的值。

```EXPRESS
TYPE IfcObjectPlacement = SELECT (
IfcLocalPlacement,
IfcGridPlacement);
END_TYPE;
```

### 2. IFC中的数据表达

#### 2.1 几何表达

IFC 构件实体的各类属性都是从上级实体中一层层继承的，如图所示。IfcBuildingElementProxy 实体的属性分别从 IfcRoot、 IfcObjectDefinition、 IfcObject、IfcProduct、 IfcElement、 IfcBuildingElement 实体中继承，该实体自身的属性只有CompositionType。其中，从图中可见， IfcBuildingElementProxy 的几何信息采用Representation 属性描述，该属性从 IfcProduct 中继承得到，其它构件实体也是通过该属性表达几何信息。

从图 2-11 中可见， Representation 属性由 IfcProductionRepresentation 表达，而 IfcProductionRepresentation 实 体 的 两 个 子 类 为 IfcProductDefinitionShape 、IfcMaterialDefinitionRepresentation，前者用于表达构件几何形状。进一步分析IfcProductDefinitionShape 的属性，可得图 2-12 所示的实体引用关系。通过层层引用，构件的几何形状由 IfcShapeRepresentation.Items 中的 IFC 实体集合表达。该分析与实际项目的 IFC 模型文件的引用关系是一致的。图 2-12 的下半部分展示了本 Model-B 中梁构件部分的 IFC 几何表达语句。

![构件几何信息表达](https://img.ironmanzzm.top/blog-img/20231215212434.png)

在 IfcShapeRepresentation 的属性中， RepresentationIdentifier 属性用于定义几何形状的识别符，而 RepresentationType 则进一步提供不同形状的表达类型。

![RepresentationIdentifier和RepresentationType定义的部分类型](https://img.ironmanzzm.top/blog-img/20231215212656.png)
