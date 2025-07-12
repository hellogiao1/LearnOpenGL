# SSAO(屏幕空间环境光遮蔽)原理与代码详解

这段代码实现了**屏幕空间环境光遮蔽(SSAO)** 效果，这是现代游戏引擎中用于增强场景深度感和真实感的关键技术。作为小白，我会用最直观的方式解释代码逻辑和底层原理。

## 整体原理
SSAO的核心思想：**模拟物体缝隙、角落和凹陷处因光线难以到达而产生的阴影效果**。想象桌子下面的空间比桌面更暗，这就是环境光遮蔽效果。

### 为什么叫"屏幕空间"？
- 不需要场景的完整3D信息
- 仅使用当前屏幕渲染的位置和法线信息
- 直接在2D屏幕空间计算遮蔽效果

## 代码逐行解析

### 1. 输入准备
```glsl
// 从G-buffer获取当前像素的3D位置(视图空间)
vec3 fragPos = texture(gPosition, TexCoords).xyz;

// 从G-buffer获取当前像素的法线向量
vec3 normal = normalize(texture(gNormal, TexCoords).rgb);

// 采样随机噪声纹理(用于随机旋转采样方向)
vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
```
- `gPosition`：存储每个像素在3D空间中的位置
- `gNormal`：存储每个像素的法线方向
- `texNoise`：随机噪声纹理，用于打破采样模式的规律性

### 2. 创建TBN矩阵(关键!)
```glsl
// 计算切线向量(与法线垂直的随机方向)
vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));

// 通过叉积计算副切线
vec3 bitangent = cross(normal, tangent);

// 构建切线空间矩阵
mat3 TBN = mat3(tangent, bitangent, normal);
```
**为什么需要TBN矩阵？**  
我们需要在当前像素周围**法线指向的半球空间**内随机采样。TBN矩阵将世界空间的采样点转换到以法线为Z轴的局部坐标系。

### 3. 核心遮蔽计算
```glsl
float occlusion = 0.0;
for(int i = 0; i < kernelSize; ++i)
{
    // 1. 将采样点转换到视图空间
    vec3 samplePos = TBN * samples[i]; // 切线空间→视图空间
    samplePos = fragPos + samplePos * radius; // 以当前像素为中心，半径为radius的球内采样
    
    // 2. 将采样点投影回屏幕空间
    vec4 offset = vec4(samplePos, 1.0);
    offset = projection * offset; // 视图空间→裁剪空间
    offset.xyz /= offset.w; // 透视除法(裁剪空间→标准化设备坐标)
    offset.xy = offset.xy * 0.5 + 0.5; // [-1,1]→[0,1](纹理坐标)
    
    // 3. 获取采样点对应的实际深度值
    float sampleDepth = texture(gPosition, offset.xy).z;
    
    // 4. 范围检查(避免远处物体影响)
    float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
    
    // 5. 判断是否被遮挡
    occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
}
```

#### 遮蔽判断图解：
```
      采样点
        │
        │ 实际几何表面
        ▼
        │ sampleDepth (实际深度)
        │
当前像素─┼──── samplePos.z (采样点理论深度)
fragPos │
        │
        │
```
- 如果 `sampleDepth < samplePos.z` → 采样点"在空中" → 不被遮挡
- 如果 `sampleDepth >= samplePos.z + bias` → 采样点被几何体遮挡

### 4. 结果计算
```glsl
// 计算平均遮蔽率
occlusion = 1.0 - (occlusion / kernelSize);

// 输出最终遮蔽值(0=完全遮蔽, 1=无遮蔽)
FragColor = occlusion;
```

## 关键参数说明

| 参数 | 作用 | 典型值 |
|------|------|--------|
| `kernelSize` | 采样点数量 | 32-64 |
| `radius` | 采样半径 | 0.5-1.0 |
| `bias` | 深度比较偏移 | 0.025 |
| `noiseScale` | 噪声纹理缩放 | 屏幕分辨率/噪声图大小 |

## 原理解析：为什么这样能计算遮蔽？

1. **半球采样**  
   在当前像素的法线方向半球内随机取点（避免在物体内部采样）

2. **深度比较**  
   比较采样点的理论深度和实际场景深度：
    - 如果实际深度更浅 → 有几何体遮挡 → 计入遮蔽
    - 如果实际深度更深 → 没有遮挡 → 不计入

3. **加权平均**  
   统计所有采样点中被遮挡的比例，作为当前像素的遮蔽强度

## 效果示意图

```
      物体表面
       │
       │  ● ○ ○    ← 采样点
       │ ○ ● ○ ●  
当前像素├─●─○───●→ 有些点被遮挡(●)
       │   ○ ● ○
       │     ●
       ▼
被遮挡区域显示更暗
```

## 为什么需要随机噪声？

1. **打破规律性**  
   固定采样模式会产生带状瑕疵(banding artifacts)

2. **增加随机性**  
   使遮蔽效果更自然，模拟真实光线行为

3. **减少采样数**  
   通过随机旋转采样核，可以用较少采样点达到较好效果

## 完整工作流程

1. 渲染场景到G-buffer(存储位置和法线)
2. 对每个像素执行SSAO计算
3. 对SSAO结果进行模糊处理(减少噪声)
4. 将遮蔽因子应用于最终光照计算

通过这段代码，游戏引擎能在屏幕空间高效模拟复杂的环境光遮蔽效果，极大增强场景的深度感和真实感，而性能消耗相对较低。