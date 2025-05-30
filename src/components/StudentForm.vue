<!-- 展示表单，接收用户输入，发出添加/更新事件  -->

<template>
  <!-- 学生信息表单，包含添加和更新学生功能 -->
  <form @submit.prevent="onSubmit">
    <input v-model="localStudent.sno" placeholder="学号" required type="number" />
    <input v-model="localStudent.name" placeholder="姓名" required />
    <input v-model.number="localStudent.age" placeholder="年龄" required type="number" />
    
    <select v-model="localStudent.gender">
      <option>男</option>
      <option>女</option>
    </select>

    <select v-model="localStudent.major">
      <option>通信工程</option>
      <option>人工智能</option>
      <option>电子信息工程</option>
      <option>自动化</option>
    </select>

    <button class="submit" type="submit">
      {{ editIndex !== null ? '更新学生' : '添加学生' }}
    </button>
    <!-- <button type="reset">重置</button> -->
  </form>
</template>

<script>
export default {
  // 组件接收两个 props 参数：newStudent 和 editIndex
  props: ['newStudent', 'editIndex'],
  data() {
    // 初始化 localStudent 对象，包含学生的基本信息字段
    return {
      localStudent: {
        sno: '',
        name: '',
        age: '',
        gender: '男',
        major: '通信工程'
      }
    }
  },
  watch: {
    // 监听 newStudent 对象的变化，当其变化时更新 localStudent 对象
    newStudent: {
      immediate: true,
      deep: true,
      handler(newVal) {
        this.localStudent = { ...newVal }
      }
    }
  },
  methods: {
    // 表单提交事件处理函数，根据 editIndex 的值决定是添加还是更新学生信息
    onSubmit() {
      if (this.editIndex !== null) {
        this.$emit('update', this.localStudent)
      } else {
        this.$emit('add', this.localStudent)
      }
    }
  }
}
</script>

<style scoped>  /* 只对本组件生效
/* 表单样式，设置表单的底部外边距 */
form {
  margin-bottom: 20px;
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(160px, 1fr));
  gap: 15px;
}
/* 输入框和选择框的样式，设置它们的右边距和内边距 */
input, select {
  margin-right: 10px;
  padding: 5px;
  border: 1px solid #ccc;
  border-radius: 6px;
  font-size: 14px;
  width: 100%;
}

/* 更新、编辑学生按钮  */
.submit {
background-color: #3498db;
  color: white;
}
/* 提交按钮的样式，设置其内边距 */
button {
  margin: 0 5px;
  padding: 8px 16px;
  border: none;
  border-radius: 6px;
  cursor: pointer;
  font-weight: bold;
}
</style>