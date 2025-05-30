<!-- 展示学生数据表格，触发删除/编辑操作 -->

<template>
  <!-- 学生列表展示 -->
  <table border="1" width="100%">
    <thead>
      <tr>
        <th>学号</th>
        <th>姓名</th>
        <th>年龄</th>
        <th>性别</th>
        <th>专业</th>
        <th>操作</th>
      </tr>
    </thead>
    <tbody>
      <!-- 遍历学生数组并为每个学生生成一行 -->
      <tr v-for="(student, index) in students" :key="student.id">
        <td>{{ student.sno }}</td>
        <td>{{ student.name }}</td>
        <td>{{ student.age }}</td>
        <td>{{ student.gender }}</td>
        <td>{{ student.major }}</td>
        <td>
          <!-- 触发编辑事件 -->
          <button class="edit" @click="$emit('edit', index)">编辑</button>
          <!-- 触发删除事件 -->
          <button class="delete" @click="$emit('delete', index)">删除</button>
        </td>
      </tr>
    </tbody>
  </table>
</template>

<script>
// 定义 StudentList 组件
export default {
  // 从父组件接收 students 数组作为 prop
  props: ['students']
}
</script>

<style scoped>
/* 设置表格样式 */

table {
  width: 100%;
  border-collapse: collapse;
  margin-top: 10px;
  background: white;
  border-radius: 8px;
  overflow: hidden;
}
/* 设置表头和单元格的内边距与文本对齐方式 */


td {
  border: 1px solid #e0e0e0;
  padding: 10px;
  text-align: center;
  font-size: 14px;
}

th {
  padding: 10px;
  background-color: #2c3e50;
  color: white;
}
/* 设置按钮的间距 */
button {
  margin: 0 5px;
  padding: 8px 16px;
  border: none;
  border-radius: 6px;
  cursor: pointer;
  font-weight: bold;
}

.edit {
  background-color: #f39c12;
  color: white;
}

.delete {
  background-color: #e74c3c;
  color: white;
}
</style>
