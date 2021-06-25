# littlePS
基于qt的类PS项目littlePS。

本项目旨在设计一款便于使用的图像编辑和处理软件。简单易用，同时有一键处理图像功能，如图像增强、各种丰富的滤镜、综合图像处理特效、基于深度学习的超分辨率（SR）图像重构等功能。要求具备图层管理、简单几何操作、导入导出、图像处理滤镜等功能，满足日常图片处理的需求。

系统分为三个模块：基本功能、特殊图像处理、基于深度学习的图像处理。



### 架构说明：

![image-20210625110239846](img\image-20210625110239846.png)

Model：保存图层数据，当前画笔等参数

ViewModel：将model中的各个图层整合成一张可显示的图片

ViewUI：布局显示ViewModel整合完毕的图片

### 功能模块

![image-20210625110305296](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625110305296.png)



1. **文件读取、文件存储、工程读取、工程存储**

2. **基本图形绘制、直线、矩形、椭圆**

3. **对选取内容的平移、旋转与缩放**

4. **对编辑内容的撤销与重做**

5. **图层操作：包括图层选取、图层上移、图层下移、图层删除以及合并全部图层**

6. **滤镜功能：实现包括反色、直方图平均、自动亮度适应、通用卷积变换、拉普拉斯图像增强、灰阶化、二值化、亮度调整以及双边滤波等滤镜效果**

7. **风格滤镜功能：实现包括怀旧、流年、光照、积木、连环画、暗调、熔铸、霓虹、曝光、冰冻以及油画等风格滤镜**

8. **超分辨率：基于不同深度学习模型（包括FSRCNN、EDSR、ESPCN以及lapSRN）实现的超分辨率重建功能**





### **超分辨率**



#### **数据集**

##### DIV2K

DIV2K：数据集有1000张高清图(2K分辨率)，其中800张作为训练，100张作为验证，100张作为测试。

EDSR、FSRCNN、ESPCN的SR训练，用到 X2、X3、X4、以及原始 train 和 Validation 数据。

LapSRN用到 X2、X4 、X8以及原始 train 和 Validation 数据。

下载地址(3.3G)：data.vision.ee.ethz.ch/cvl/DIV2K/DIV2K_train_HR.zip



### **Requirements**

• Tensorflow

• Python

• Numpy

• cv2

• imutils

#### FSRCNN：

 FSRCNN主要的目的是加速SRCNN模型。重新设计SRCNN结构，主要 在三个方面：一是使用了一个解卷积层在最后，这个作用是从没有差值的低分辨率图像直接映射到高分辨率图像。第二是，重新改变输入特征维数。第三是使用了更小的卷积核但是使用了更多的映射层。针对SRCNN中有两点限制了速度。第一点，低分辨率图像需要上采样（通过三次插值）；第二点，非线性映射步骤，需要缩减参数加快速度。对于第一个问题采用解卷积层代替三次插值，针对第二个问题，添加萎缩层和扩张层，并将一个大层用一些小层（卷积核大小是3*3）来代替。整个网络结构类似于漏斗的形状，中间细两端粗。这个网络不仅仅速度快，而且不需要更改参数除了最后一个解卷积层。三个贡献：1、设计漏斗结构的卷积网络，不需要预处理操作2、速度提升3、训练速度快，只要改变最后的解卷积层就可以。

![image-20210625111013923](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625111013923.png)



•x2, x3, x4 trained models available

•优势：快速，小巧而准确

•缺点：艺术表现上不够精确

•batch size : 2

•Iterations:30

•每个Epoch训练图片数量:60



#### EDSR：

 EDSR借鉴了ResNet网络基于残差进行学习的机制，input经过一层卷积后兵分两路，一条路经过n层的ResBlock再卷积一次，一条路则直接通向交汇处，进行加权求和，再经过上采样的处理与卷积输出结果。但是EDSR与ResNet中的ResBlock有些不同，EDSR中没有BN层，没有标准化的过程。

![image-20210625111030508](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625111030508.png)

•x2, x3, x4 trained models available

•优势：高度准确

•缺点：一个字慢

•batch size : 16

•Iterations:50

•每个Epoch训练图片数量:800



#### ESPCN：

 像SRCNN那样的方法，由于需要将低分辨率图像通过上采样插值得到与高分辨率图像相同大小的尺寸，再输入到网络中，这意味着要在较高的分辨率上进行卷积操作，从而增加了计算复杂度。ESPCN直接在低分辨率图像尺寸上提取特征，计算得到高分辨率图像的高效方法。ESPCN网络结构如下图所示。

![image-20210625111046350](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625111046350.png)

•x2, x3, x4 trained models available

•优点：它体积小，速度快，并且仍然表现良好。

•缺点：视觉效果更差。

•batch size : 16

•Iterations:50

•每个Epoch训练图片数量:800



#### LapSRN：

1.**级联结构（金字塔结构）**：网络有两个分支，1个是特征提取分支，1个是图像重构分支

 2.提出一种新的**loss**函数

![image-20210625111125124](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625111125124.png)

LapSRN结构图：

![image-20210625111122242](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625111122242.png)

•x2, x4, x8 trained models available

•优点：该模型可以通过一次正向传递进行多尺度超分辨率。

•缺点：它比ESPCN和FSRCNN慢，并且精度比EDSR差。

•batch size : 16

•Iterations:50

•每个Epoch训练图片数量:800



#### **SR集成项目**

OpenCV中的dnn（Deep Neural Network module）模块是专门用来实现深度神经网络相关功能的模块。OpenCV自己并不能训练神经网络模型，但是它可以载入别的深度学习框架（例如TensorFlow）训练好的模型。而且OpenCV在载入模型时会使用自己的dnn模块对模型重写，使得模型的运行效率更高。



正好本项目采用Qt， 用cmake编译opencv库以及contrib dnn模块得到动态链接库就行，接口：

```c++
DnnSuperResImpl sr; //Dnn超分辨率实例
sr.readModel(/*模型路径*/);
sr.setModel(/*模型名称, 倍数*/);
```



### **界面展示**(仅部分)

![image-20210625110657948](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625110657948.png)

#### **绘图基本功能：基本图形绘制**

![image-20210625110712793](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625110712793.png)

根据预设的填充样式与边框色绘制的矩形



![image-20210625110722544](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625110722544.png)

实现基本图形的绘制



#### **绘图基本功能：平移、旋转、缩放**

![image-20210625110743886](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625110743886.png)

![image-20210625110748319](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625110748319.png)

![image-20210625110751583](D:\workspace\miniPS\miniPS\LittlePS\img\image-20210625110751583.png)

#### 

### 

