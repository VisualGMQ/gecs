# GECS

`gecs` 是一个参考了[EnTT](https://github.com/skypjack/entt)源码结构，和[Bevy-ECS](https://bevyengine.org/)API的用于游戏开发的ECS系统。采用C++17。

目前在msvc下编译通过。

## 使用方法

### 基本例子

`gecs`的API大量借鉴了`bevy`游戏引擎，下面是一个简单例子：

```cpp
// 包含头文件
#include "gecs/gecs.hpp"

using namespace gecs;

// 一个component类型
struct Name {
    std::string name;
};

// 一个resource类型
struct Res {
    int value;
};

// 每帧都会更新的system
void update_system(commands cmds, querier<Name> querier, resource<Res> res) {
    for (auto& [_, name] : querier) {
        std::cout << name.name << std::endl;
    }

    std::cout << res->value << std::endl;
}

int main() {
    world world;

    // 得到Lambda对应的函数指针
    constexpr auto startup = +[](commands cmds) {
        auto entity1 = cmds.create();
        cmds.emplace<Name>(entity1, Name{"ent1"});
        auto entity2 = cmds.create();
        cmds.emplace<Name>(entity2, Name{"ent2"});

        cmds.emplace_resource<Res>(Res{123});
    };

    // 注册这个函数指针
    world.regist_startup_system<startup>();

    // 使用普通函数
    world.regist_update_system<update_system>();
    // 使用函数指针也可
    // world.regist_update_system<&update_system>();

    world.startup();
    world.update();

    return 0;
}
```

### world

`world` 是整个ECS的核心类，管理几乎所有ECS数据。

使用默认构造函数创建一个即可：

```cpp
gecs::world world;
```

一个典型的ECS程序一般如下：

```cpp
// 创建world
gecs::world world;

// 注册startup system
world.regist_starup_system<your_startup_system1>();
world.regist_starup_system<your_startup_system2>();
...

// 注册update system 
world.regist_update_system<your_update_system1>();
world.regist_update_system<your_update_system2>();
...

// 启动ECS
world.startup();

// 游戏循环中每帧更新ECS
while (shouldClose()) {
    world.update();
}
```

### system

`system`分为两种：
* `startup system`：在启动时执行一次，主要用于初始化数据
* `update system`：每帧运行一次

`system`**不是**`std::function`类型，而是普通函数类型。所以若想使用lambda，则不能有任何捕获。

`startup system`有固定的函数类型`void (*)(commands)`，使用`regist_startup_system`即可注册：

```cpp
world.regist_startup_system<your_startup_system>();
```

`update system`没有固定函数类型，但只能包含零个或多个`querier`/`resource`，以及零个或一个`commands`。参数顺序没有要求。使用`regist_update_system`即可注册:

```cpp
// 使用lambda，无捕获的lambda会被转换为普通函数，在lambda前面加`+`可以获得对应函数类型
// 含有两个querier和一个resource
world.regist_update_system<+[](querier<Name> q1, querier<Family> q2, resource<FamilyBook> res)>();
// 含有一个commands和一个q1
world.regist_update_system<+[](commands cmd, querier<Name> q1)>();
```

### querier和resource

`querier`用于从`world`中查询拥有某种组件的实体，一般作为`system`的参数：

```cpp
// q1查询所有含有Name实体的组件，q2查询所有函数Family实体的组件。并且Name组件不可变，Family可变
void update_system(querier<Name> q1, querier<mut<Family>> q2) { ... }
```

只有使用`mut<T>`模板包裹组件类型时，才能够得到可变组件。这是为了之后对各个系统进行并行执行打下基础。

可以直接遍历`querier`来得到所有实体和对应组件：

```cpp
for (auto& [entity, name] : q1) {
    ...
}

// 组件很多时按querier类型中声明的顺序得到
for (auto& [entity, comp1, comp2, comp3] : multi_queirer) {
    ...
}
```

`resource`则是对资源的获取。资源是一种在ECS中唯一的组件：

```cpp
void system(resource<Name> res) {
    // 通过operator->直接获得。不存在资源会导致程序崩溃！
    res->name = "ent";
}
```

### commands

`commands`是用于向`world`中添加/删除实体/组件/资源的类：

```cpp
void system(commands cmds) {
    // 创建entity
    auto entity = cmds.create();
    // 附加组件到实体
    cmds.emplace<Name>(entity, Name{"ent"});

    // 从实体上删除组件
    cmds.remove<Name>(entity);

    // 删除实体及其所有组件
    cmds.destroy(entity);

    // 设置资源
    cmds.emplace_resource<Res>(Res{});

    // 移除并释放资源
    cmds.remove_resource<Res>();
}
```
