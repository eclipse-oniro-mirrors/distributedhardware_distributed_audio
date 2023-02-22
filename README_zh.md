# **分布式音频部件**

## **简介**

分布式音频是多个设备的音频同时协同使用的能力。分布式音频部件是为分布式硬件子系统提供这一能力的部件。本部件不直接对接应用，只向分布式硬件框架子系统提供C++接口。应用可以通过音频框架的接口使用分布式音频部件操作其他设备的音频设备，使用方式与本地音频一致。

其系统架构图如下图所示：

![](figures/distributedaudio_arch.png)

**分布式音频框架实现(DistributedAudioFwkImpl)**：为分布式硬件管理框架提供分布式音频初始化、释放、使能、去使能能力，以及音频设备配置参数能力。

**分布式音频主控端管理模块(DAudioSourceManager)**：管理和创建主控端Fwk层分布式音频设备实体。

**分布式音频被控端管理模块(DAudioSinkManager)**：管理和创建被控端Fwk层分布式音频设备实体，以及本地设备信息。

**主控端分布式音频设备的抽象实体(DAudioSourceDevice)**：被控端音频设备在主控端的抽象，拥有被控端音频的参数属性，实现被控端音频设备音量、焦点、媒体键事件的控制。执行录音功能时，接收从被控端传来的音频编码数据，完成解码操作后，送给主控端音频框架pcm流。当知执行放音功能时，接收主控端音频框架采集音频pcm流，进行编码处理后，将音频编码数据传送给被控端。

**被控端分布式音频源设备的抽象实体(DAudioSinkDevice)**：作为主控端音频设备在被控端的代理，实现对被控端音频设备音量、焦点、媒体键事件的直接响应和控制。当执行录音功能时，接收被控端音频框架采集音频pcm流，进行编码处理后，将音频编码数据传送给主控端。当知执行放音功能时，接收从主控端传来的音频编码数据，完成解码操作，送给被控端音频框架pcm流播放。

**分布式音频主控端控制管理模块(DAudioSourceCtrlMgr)**：负责响应被控端媒体键事件、主控端与被控端之间音量同步、主控端远程控制被控端设备音量、响应被控端音频焦点状态等任务的处理。

**分布式音频被控端控制管理模块(DAudioSinkCtrlMgr)**：负责监听被控端的设备音量、音频焦点状态、媒体键事件，执行主控端音量调节指令、反馈被控端音频焦点状态和被控端媒体键事件。

**分布式音频被控端传输处理模块(DAudioTransport)**：负责主控端和被控端音频数据的处理和传输，包括音频编码、音频解码、音频数据发送、音频数据接收等操作。

**分布式音频被控端代理模块(DAudioSinkClient)**：与被控端音频框架交互，完成音频pcm流的播放或者采集。

**HDF分布式音频设备管理扩展模块(DAudio Manager)**：负责分布式音频驱动与分布式音频服务间的交互，包括设备注册、去注册、打开、关闭等。

**HDF分布式音频设备管理模块(Audio Manager)**：负责创建和管理HDF层分布式音频设备实体，与音频框架跨进程交互，通知设备上下线状态。

**HDF分布式音频设备驱动实体(Audio Adapter)**：作为驱动层分布式音频设备的抽象实体，描述被控端音频设备在主控端的驱动对象。


## **目录**

```
/foundation/distributedhardware/distributed_audio
├── audio_handler                          # 分布式音频硬件信息上报、设备状态变化通知，由分布式硬件管理框架加载
├── common                                 # 分布式音频公共模块
├── interfaces                             # 分布式音频对外接口模块
├── sa_profile                             # 分布式音频SA配置模块
├── hdf_interface                          # 分布式音频hdf接口模块
├── hdf_service                            # 分布式音频hdf服务模块
├── services                               # 分布式音频服务模块
│   ├── audioclient                        # 分布式音频客户端
│   ├── audiocontrol                       # 分布式音频控制管理模块
│   ├── audiohdiproxy                      # 分布式音频HDI代理模块
│   ├── audiomanager                       # 分布式音频服务管理模块
│   ├── audioprocessor                     # 分布式音频数据处理模块，包括编码，解码等
│   ├── audiotransport                     # 分布式音频数据传输组件
│   ├── common                             # 分布式音频服务公共模块
│   └── softbusadapter                     # 软总线接口适配器，为音频事件传输提供统一传输接口
```

## **约束**
**语言限制**：C++语言。  
**组网环境**：必须确保设备在同一个局域网中。  
**操作系统限制**：OpenHarmony操作系统。  

## **说明**
### **概念说明**
主控端（source）：控制端，通过调用分布式音频能力，使用被控端的音频播放，录音等功能。
被控端（sink）：被控制端，通过分布式音频接收主控端的命令，使用本地音频为主控端提供音频数据。

### **接口说明**
分布式音频部件实现分布式硬件管理框架提供的接口，分布式硬件管理框架统一调用接口实现虚拟硬件驱动注册等功能。

### **场景说明**
被控端设备上线之后，主控端可以使能该设备音频并像使用本地音频一样使用被控端音频，直到被控端设备下线。

### **流程说明**
#### **1. 设备开机启动**
系统拉起分布式音频的SA服务，Source侧被初始化，相关模块被初始化。

#### **2. 设备组网上线**
设备上线后，分布式硬件管理框架同步到上线设备的音频硬件信息并使能，使能成功后在系统中会新增分布式音频驱动并通知到音频框架，音频框架统一管理本地音频和分布式音频驱动；上层应用通过音频框架接口可以查询到分布式音频，并按照和本地音频相同的接口使用分布式音频。

#### **3. 设备下线**
设备下线后，分布式硬件管理框架去使能下线设备的音频硬件，本地移除分布式音频驱动并通知到音频框架，此时下线设备的分布式音频不可用。

## **相关仓**
****
**分布式硬件子系统：**

设备管理
[device_manager](https://gitee.com/openharmony/distributedhardware_device_manager)

分步式硬件管理框架
[distributed_hardware_fwk](https://gitee.com/openharmony/distributedhardware_distributed_hardware_fwk)

分布式相机
[distributed_camera](https://gitee.com/openharmony/distributedhardware_distributed_camera)

分布式屏幕
[distributed_screen](https://gitee.com/openharmony/distributedhardware_distributed_screen)

**分布式音频**
[distributed_audio](http://mgit-tm.rnd.huawei.com/hmf/distributedhardware/distributed_audio)
