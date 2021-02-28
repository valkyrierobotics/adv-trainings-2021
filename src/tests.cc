#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <vector>

// TEST(TestSuite, TestName)

TEST(Basic, AlwaysPass) {}

TEST(Basic, AlwaysPassExpect) { EXPECT_EQ(1, 1); }

TEST(Basic, DISABLED_AlwaysFailExpect) {
  EXPECT_EQ(1, 2) << "Why did we think 1 and 2 were equal?";
}

TEST(Basic, DISABLED_AlwaysFail) { FAIL() << "This test always fails"; }

TEST(Basic, ExpectVsAssert) {
  int *danger = new int(5);
  ASSERT_NE(danger, nullptr);
  EXPECT_EQ(*danger, 5);
  delete danger;
}

// A stack is a Last-In First-Out data structure
// Operations: push (adds something) pop (removes something)
//
// Linked list: list of connected nodes, a node holds data and
// pointer to the next node
//
// head              tail
// v                 v
// 5 --> 6 --> 7 --> 8
//
// push():
//     new_node = Node{head, data};
//     head = new_node
//
// pop():
//     to_return = head.data
//     head = head.next
//
// std::unique_ptr<int>
//
// template <typename T>
// class unique_ptr {
//    unique_ptr(const unique_ptr&) = delete;
//
//    unique_ptr(unique_ptr &&other) {
//       data_ = other.data_;
//       other.data_ = nullptr;
//    }
//
//    ~unique_ptr() {
//       if (data_) {
//         delete data_;
//       }
//    }
// }
//
// RAII (Resource acquisition is initialization) wrapper around a raw pointer
//

class Stack {
public:
  Stack() {}

  //                   tail
  //                   v
  // 5 --> 6 --> 7 --> 8
  // ^
  // 12 < head
  void push(std::unique_ptr<int> data) {
    auto new_node = std::make_unique<Node>(std::move(head_), std::move(data));
    head_ = std::move(new_node);
  }

  std::unique_ptr<int> pop() {
    if (!head_) {
      return nullptr;
    }

    auto ret = std::move(head_->data_);
    head_ = std::move(head_->next_);

    return std::move(ret);
  }

  int size() const {
    auto ret = 0;
    for (auto curr = head_.get(); curr; curr = curr->next_.get()) {
      ret++;
    }
    return ret;
  }

private:
  struct Node {
    Node(std::unique_ptr<Node> next, std::unique_ptr<int> data)
        : next_{std::move(next)}, data_{std::move(data)} {}

    std::unique_ptr<Node> next_;
    std::unique_ptr<int> data_;
  };

  std::unique_ptr<Node> head_{nullptr};
};

class StackTest : public ::testing::Test {
protected:
  void SetUp() {
    stack_.push(std::make_unique<int>(5));
    stack_.push(std::make_unique<int>(6));
  }

  Stack stack_;
  Stack empty_stack_;
};

TEST_F(StackTest, SizeWorks) {
  EXPECT_EQ(stack_.size(), 2);
  EXPECT_EQ(empty_stack_.size(), 0);
}

TEST_F(StackTest, PopWorks) {
  auto rem = empty_stack_.pop();
  EXPECT_EQ(rem, nullptr);

  rem = stack_.pop();
  ASSERT_NE(rem, nullptr);
  EXPECT_EQ(*rem, 6);

  rem = stack_.pop();
  ASSERT_NE(rem, nullptr);
  EXPECT_EQ(*rem, 5);

  rem = stack_.pop();
  EXPECT_EQ(rem, nullptr);
}

TEST_F(StackTest, PushWorks) {
  empty_stack_.push(std::make_unique<int>(5));
  EXPECT_EQ(empty_stack_.size(), 1);

  auto ret = empty_stack_.pop();
  ASSERT_NE(ret, nullptr);
  EXPECT_EQ(*ret, 5);
  EXPECT_EQ(empty_stack_.size(), 0);
}
