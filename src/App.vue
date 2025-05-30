<!-- 管理学生列表数据、操作逻辑、负责连接子组件 -->

<template>
  <div class="app">
    <h1>🎓 学生信息管理系统</h1>
    <!-- 学生信息表单，用于添加或编辑学生信息 -->
    <StudentForm
      :newStudent="newStudent"  
      :editIndex="editIndex"
      @add="addStudent"
      @update="updateStudent" 
    /> <!-- 传递对象并定义监听事件  -->
    <!-- 输入框，用于搜索学生姓名 -->

    <input class="search-bar" v-model="keyword" placeholder="🔍 搜索学生姓名..." />
    <!-- 学生信息列表，用于展示学生信息并提供编辑和删除功能 -->
    <StudentList
      :students="filteredStudents"
      @delete="deleteStudent"
      @edit="editStudent"
    />
  </div>
</template>

<script>
import StudentForm from './components/StudentForm.vue'
import StudentList from './components/StudentList.vue'

export default {
  components: { StudentForm, StudentList }, // 注册子组件
  data() {
    return {
      students: [ // 学生信息数组
        {
          id: 1,
          sno: "202305571104",
          name: "张三",
          age: 18,
          gender: "男",
          major: "通信工程"
        },
        {
          id: 2,
          sno: "202305571166",
          name: "李四",
          age: 20,
          gender: "女",
          major: "人工智能"
        }
      ],
      newStudent: { // 新学生对象，用于表单数据绑定
        sno: "",
        name: "",
        age: "",
        gender: "男",
        major: "通信工程"
      },
      editIndex: null, // 当前编辑的学生索引
      keyword: "" // 搜索关键字
    }
  },
  computed: {
    // 根据关键字过滤学生信息
    filteredStudents() {
      return this.students.filter(s =>
        s.name.includes(this.keyword)
      )
    }
  },
  methods: {
    // 添加新学生
    addStudent(student) {
      const id = Date.now() // 返回自1970年1月1日00:00:00 UTC以来的毫秒数
      this.students.push({ ...student, id }) // 将传入的student对象的属性复制到新的对象中，并添加一个id属性
      this.newStudent = {  // 重置新学生对象
        sno: "",
        name: "",
        age: "",
        gender: "男",
        major: "通信工程"
      }
    },
    // 删除学生信息
    deleteStudent(index) {
      this.students.splice(index, 1) // 开始修改的数组索引位置, 删除一个元素
    },
    // 编辑学生信息
    editStudent(index) {
      this.newStudent = { ...this.students[index] } // 赋值要修改学生对象属性赋值给newstudent对象
      this.editIndex = index // 记录当前编辑的学生对象索引, 以方便更新
    },
    // 更新学生信息
    updateStudent(student) {
      this.students[this.editIndex] = { ...student }
      this.newStudent = {
        sno: "",
        name: "",
        age: "",
        gender: "男",
        major: "通信工程"
      }
      this.editIndex = null
    }
  }
}
</script>

<style>
 body {
  font-family: "微软雅黑", sans-serif;
  background-color: #f9f9f9;
  margin: 0; 
  padding: 0;
}

/* .container {
  max-width: 900px;
  margin: 30px auto;
  padding: 20px;
  background: white;
  border-radius: 12px;
  box-shadow: 0 0 12px rgba(0, 0, 0, 0.05);
} */

h1 {
  text-align: center;
  color: #2c3e50;
  margin-bottom: 20px;
  font-weight: bold;
}

/* 搜索框 */
.search-bar {
  margin-bottom: 15px;
  width: 100%;
  padding: 8px;
  font-size: 14px;
  border: 1px solid #ccc;
  border-radius: 6px;
}

</style>

