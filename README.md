# 🎓 学生信息管理系统（Vue3 + Options API）

本项目是一个基于 Vue3 的前端练习小项目，主要实现了学生信息的增删改查功能（CRUD），采用 JSON 模拟后端数据。适合前端初学者练习 Vue 基础与组件设计思路。
前端使用 Vue3 + Options API （选项式）实现，数据部分采用 JSON（对象数组） 模拟，暂不接入后端数据库。


页面分为三个模块：信息录入表单、搜索栏和学生信息表格展示。学生信息包含学号、姓名、年龄、性别和专业，专业和性别使用下拉框限定输入。目前页面已经实现了所有核心功能，数据是保存在浏览器本地中。用户可以在表单中填写信息进行新增，也可以点击编辑按钮加载数据进行修改，删除和模糊搜索也都已经完成。
  

• 为保证页面刷新后数据不丢失，因此采用了localstorage来进行一个浏览器本地保存，后续可以通过连接数据库解决该问题。

• 表单目前缺乏编辑撤销的交互，只能通过刷新页面来实现，这个地方之后可以再完善一下，后续可以考虑增加一个‘取消编辑’按钮，点了之后会清空输入并返回新增模式，这样用户体验更完整，也避免误操作。

• 目前还没接入真实后端，但考虑到后续可以用本地存储或 mock 接口来模拟请求逻辑。另外样式方面做了基础美化，风格偏向学院风，后期也可以尝试接入 UI 框架进一步优化体验。

📍 项目地址：https://github.com/GLDDY/SCRUD

---

## ✨ 项目功能

- ✅ 显示学生信息列表（表格形式）
- ➕ 添加学生信息
- 📝 编辑学生信息（支持取消编辑）
- ❌ 删除学生信息
- 🔍 按姓名关键字搜索
- 🎨 页面初步美化，适合“学院风”展示风格

---

## 📚 学生信息字段

| 字段   | 类型   | 说明                         |
|--------|--------|------------------------------|
| 学号   | String | 必填，唯一标识               |
| 姓名   | String | 必填                         |
| 年龄   | Number | 必填                         |
| 性别   | String | 男 / 女                      |
| 专业   | String | 通信工程 / 人工智能 / 电子信息工程 / 自动化 |

---

## 🛠 技术栈

- [Vue 3](https://vuejs.org/)（基于 Options API）
- HTML / CSS / JavaScript
- JSON 模拟本地数据
- Vue Cli 构建工具

---

## 🖼️ 页面截图

### 📋 学生信息展示界面
![首页截图](./public/screenshots/首页.png)

### ➕ 添加学生界面
![添加学生](./public/screenshots/添加1.png)
![添加学生](./public/screenshots/添加2.png)

### 📝 编辑状态界面
![编辑学生](./public/screenshots/编辑1.png)
![编辑学生](./public/screenshots/编辑2.png)

### ❌ 删除功能演示
![删除功能](./public/screenshots/删除.png)

### 🔍 搜索功能效果
![搜索功能](./public/screenshots/搜索.png)

---

## 🚀 快速启动

```bash
# 克隆项目
git clone https://github.com/GLDDY/SCRUD.git

# 进入目录
cd SCRUD

# 安装依赖
npm install

# 启动项目
npm run dev
```

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.
