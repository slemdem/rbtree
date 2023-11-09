#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "rbtree.h"

// rbtree 선언 및 초기화
rbtree *new_rbtree(void) {
  rbtree *t = (rbtree *)calloc(1, sizeof(rbtree));

  // calloc을 통한 nil 선언과 초기화
  t -> nil = (node_t *)calloc(1, sizeof(node_t));

  // nil 노드의 색은 항상 BLACK
  t -> nil -> color = RBTREE_BLACK;
  
  // root를 nil노드로 초기화 및 선언
  t -> root = t -> nil;

  return t;
}

// rbtree 삭제의 노드 삭제
void delete_rbtree_node(node_t *p){
  // nil노드 메모리는 해제하지 않는다 calloc으로 선언된 nil의 key값은 0이다
  if(p->key == 0) return;
  
  // 각노드의 왼쪽,오른쪽 노드를 먼저 순회한다(후위순회)
  delete_rbtree_node(p->left);
  delete_rbtree_node(p->right);
  // 마지막으로 노드의 메모리를 해제해준다
  free(p);

  return;
}

// rbtree 삭제
void delete_rbtree(rbtree *t) {
  // 후위순회로 노드를 제거하는 delete_rbtree_node(node_t *p) 호출
  delete_rbtree_node(t->root);
  
  // 모든 노드 제거후 nil노드 메모리 해제
  free(t->nil);
  // 트리 메모리 해제
  free(t);
}

// 노드 생성
node_t *new_node_t(rbtree *t, node_t* parent_node, const key_t key){
  node_t *p = (node_t *)malloc(sizeof(node_t));

  p -> color = RBTREE_RED;  // 삽입하는 노드는 항상 RED이다
  p -> key = key;  // 입력된 key값 할당
  p -> parent = parent_node; // 입력된 parent 설정
  // 삽입되는 노드의 하위노드는 항상 nil노드이다
  p -> left = t -> nil; 
  p -> right = t -> nil;
  
  return p;
}

// 좌회전 - 현재노드를 왼쪽아래로 보내고 오른쪽 노드를 현재위치에 올린다
void rotate_left(rbtree *t, node_t* cur_node){
  // 현재 위치에 오게될 오른쪽 노드에 접근하기 위해 rightnode 선언
  node_t * right_node = cur_node->right;

  // 현재노드의 오른쪽에 오른쪽 노드의 왼쪽 자식노드를 붙인다
  cur_node->right = right_node -> left;
  if (right_node->left != t->nil){
    right_node->left->parent = cur_node; // 오른쪽 노드의 왼쪽 자식노드가 nil이 아닐 때 그 자식노드의 부모를 현재노드로 바꿔준다
  }

  right_node->parent = cur_node->parent; // 오른쪽노드의 부모는 현재노드의 부모가 된다
  // 부모노드에 대한 현재 노드의 위치에 따른 parent 조작
  if (cur_node->parent == t->nil){
    t->root = right_node; //현재 노드의 부모가 nil노드 즉 현재노드는 root일 때 루트는 오른쪽 노드가 된다
  }else if(cur_node == cur_node->parent->left){
    cur_node->parent->left = right_node;  // 현재노드가 부모의 왼쪽에 존재할 때
  }else{
    cur_node->parent->right = right_node; // 현재노드가 부모의 오른쪽에 존재할 때
  }

  // 현재노드는 오른쪽 노드의 왼쪽 자식이 된다
  right_node->left = cur_node; 
  cur_node->parent = right_node;
}

// 우회전 - 현재노드를 오른쪽 아래로 보내고 왼쪽 노드를 현재위치에 올린다
void rotate_right(rbtree *t, node_t* cur_node){
  // 현재 위치에 오게될 왼쪽 노드에 접근하기 위해 leftnode 선언
  node_t * left_node = cur_node -> left;
  
  // 현재노드의 왼쪽에 왼쪽 노드의 오른쪽 자식노드를 붙인다
  cur_node -> left = left_node -> right;
  if (left_node -> right != t -> nil){
    left_node -> right -> parent = cur_node; // 왼쪽 노드의 오른쪽 자식노드가 nil이 아닐 때 그 자식노드의 부모를 현재노드로 바꿔준다
  }

  left_node->parent = cur_node->parent;// 왼쪽노드의 부모는 현재노드의 부모가 된다
  // 부모노드에 대한 현재 노드의 위치에 따른 parent 조작
  if (cur_node->parent == t->nil){
    t -> root = left_node; //현재 노드의 부모가 nil노드 즉 현재노드는 root일 때 루트는 왼쪽 노드가 된다
  }else if(cur_node == cur_node->parent->left){
    cur_node -> parent -> left = left_node;   // 현재노드가 부모의 왼쪽에 존재할 때
  }else{
    cur_node -> parent -> right = left_node;  // 현재노드가 부모의 오른쪽에 존재할 때
  }
  
  // 현재노드는 왼쪽 노드의 오른쪽 자식이 된다
  left_node -> right = cur_node;
  cur_node -> parent = left_node;
}

// 삽입후 트리 요건 만족을 위한 수정
void rbtree_insert_fixup(rbtree *t, node_t *cur_node){
  // 삽입하는 노드는 RED이기에 부모노드가 RED일 경우 RBtree의 4번 특성에 위배된다
  while (cur_node->parent->color == RBTREE_RED)
  {
    // 부모노드가 조부모 노드의 왼쪽노드일 때
    if(cur_node->parent == cur_node->parent->parent->left){
      // 삼촌노드를 접근하기위해 변수로 받아온다 조부모노드의 오른쪽
      node_t *uncle_node = cur_node->parent->parent->right;
      
      // 삼촌노드가 RED일 때
      if (uncle_node->color == RBTREE_RED){ 
        //부모노드와 삼촌노드의 색을 BLACk으로 바꾸고 조부모 노드의 색은 RED가 된다  
        cur_node->parent->color = RBTREE_BLACK;
        uncle_node->color = RBTREE_BLACK;
        cur_node->parent->parent->color = RBTREE_RED;
        // 다음 루프에서 조부모 노드의 기준에서 rbtree특성에 위배되는지 확인하게 된다
        cur_node = cur_node -> parent -> parent;
      }
      else{ // 삼촌노드가 BLACK일 때(처음에는 삼촌노드가 nil노드일 경우이다)
        // 현재노드가 부모노드의 오른쪽일 때
        if(cur_node == cur_node->parent->right){
          // 부모노드에서 좌회전 하여 일직선으로 만든다
          cur_node = cur_node->parent;
          rotate_left(t,cur_node);
        }
        // 부모노드의 색은 BLACK으로 조부모 노드의 색은 RED로 바꾸고 
        cur_node->parent->color = RBTREE_BLACK;
        cur_node->parent->parent->color = RBTREE_RED;
        // 조부모노드를 기준으로 우회전하여 부모노드가 조부모 노드에 위치하게 한다 
        rotate_right(t,cur_node->parent->parent);
      }
    }
    else{ // 부모노드가 조부모 노드의 오른쪽 노드일 때
      // 삼촌노드를 접근하기위해 변수로 받아온다 조부모노드의 왼쪽
      node_t *uncle_node = cur_node->parent->parent->left;

      // 삼촌노드가 RED일 때
      if (uncle_node->color == RBTREE_RED){
        // 부모노드와 삼촌노드의 색을 BLACk으로 바꾸고 조부모 노드의 색은 RED가 된다  
        cur_node->parent->color = RBTREE_BLACK;
        uncle_node->color = RBTREE_BLACK;
        cur_node->parent->parent->color = RBTREE_RED;
        // 다음 루프에서 조부모 노드의 기준에서 rbtree특성에 위배되는지 확인하게 된다
        cur_node = cur_node -> parent -> parent;
      }
      else{ // 삼촌노드가 BLACK일 때(처음에는 삼촌노드가 nil노드일 경우이다)
        // 현재노드가 부모노드의 왼쪽일 때
        if(cur_node == cur_node->parent->left){
          // 부모노드에서 우회전 하여 일직선으로 만든다
          cur_node = cur_node->parent;
          rotate_right(t,cur_node);
        }
        // 부모노드의 색은 BLACK으로 조부모 노드의 색은 RED로 바꾸고 
        cur_node->parent->color = RBTREE_BLACK;
        cur_node->parent->parent->color = RBTREE_RED;
        // 조부모노드를 기준으로 좌회전하여 부모노드가 조부모 노드에 위치하게 한다 
        rotate_left(t,cur_node->parent->parent);
      }
    }
  }
  // 루트의 색은 항상 BLACK이다
  t->root->color = RBTREE_BLACK;
}

// rbtree에 노드 삽입
node_t *rbtree_insert(rbtree *t, const key_t key) {
  // 부모노드로 설정할 이전노드와 새로운 노드가 삽입될 위치를 나타내는 현재노드 선언과 초기화
  node_t *past_node = t->nil;
  node_t *cur_node = t->root;

  // 현재노드가 nil노드가 될 때까지 진행된다(삽입될 위치와 그 부모를 찾는다)
  while (cur_node != t->nil)
  {
    // 확인했던 노드로 이전노드를 갱신
    past_node = cur_node; 
    // key값에 따라 확인할 노드를 현재노드로 갱신한다
    if (key < cur_node->key){
      cur_node = cur_node -> left;
    }else{
      cur_node = cur_node -> right;
    }
  }

  // 삽입할 노드를 생성한다
  node_t * newnode = new_node_t(t,past_node,key);

  // 이전노드의 key값을 통해 삽입되는 노드가 부모노드 기준에서 어디인지 알 수 있다
  if(past_node == t->nil) t->root = newnode; // 이전노드가 초기화 했던 노드값(nil노드)일경우 이 노드는 root이다
  else if(key < past_node->key) past_node->left = newnode;
  else past_node->right = newnode;
  
  // 노드를 삽입하였기에 rbtree의 규칙에 위배되는지 확인한다 
  rbtree_insert_fixup(t,newnode);

  return t->root;
}

// key값 기준의 노드탐색
node_t *rbtree_find(const rbtree *t, const key_t key) {
  // 처음에는 root노드를 방문한다
  node_t * cur_node = t->root;

  // nil 노드가 나온다면 중단
  while (cur_node != t->nil)
  {
    if(cur_node->key == key) return cur_node; // 그 전에 찾는 key값이 나온다면 해당 노드 반환
    // key값을 비교하여 방문
    else if(cur_node->key < key) cur_node = cur_node->right;
    else cur_node = cur_node->left;
  }

  // 못찾을 경우 NULL반환
  return NULL;
}

// 최소 key값의 노드 탐색 (트리의 가장 왼쪽 노드)
node_t *rbtree_min(const rbtree *t) {
  // 처음에는 root노드를 방문한다
  node_t *cur_node = t -> root;

  // 왼쪽 노드로 nil노드가 보인다면 중단한다. 아니라면 왼쪽노드를 방문한다
  while (cur_node -> left != t->nil) cur_node = cur_node->left;
  
  // nil노드가 아닌 가장 왼쪽노드를 반환한다
  return cur_node;
}

// 최대 key값의 노드 탐색 (트리의 가장 오른쪽 노드)
node_t *rbtree_max(const rbtree *t) {
  // 처음에는 root노드를 방문한다
  node_t *cur_node = t -> root;
  
  // 오른쪽 노드로 nil노드가 보인다면 중단한다. 아니라면 오른쪽 노드를 방문한다
  while (cur_node -> right != t->nil) cur_node = cur_node->right;
  
  // nil노드가 아닌 가장 오른쪽노드를 반환한다
  return cur_node;
}

// 삭제할 노드의 부모와 삭제할 노드의 자식노드의 관계 설정
void rbtree_transplant(rbtree *t, node_t *for_delete, node_t *child_node){
  // 삭제하려는 노드가 루트 노드일 때
  if (for_delete->parent == t->nil) t->root = child_node; 
  // 부모노드의 자식으로 삭제할 노드의 위치에 따라 child노드를 연결한다
  else if (for_delete == for_delete->parent->left) for_delete->parent->left = child_node; 
  else for_delete->parent->right = child_node;
  
  // child노드에 부모노드를 연결해준다
  child_node->parent = for_delete->parent;
}

// 삭제후 트리 요건 만족을 위한 수정
void rbtree_delete_fixup(rbtree *t, node_t *cur_node){
  // 이 함수가 시작되었다는 것은 현재 노드의 서브트리가 같은 레벨의 다른 서브트리보다 BlackHeight가 1 낮다는 것
  // 현재노드의 색이 BLACK일 경우 트리의 끝까지 올라갈 수 있다
  while (cur_node != t->root && cur_node->color == RBTREE_BLACK)
  {
    node_t *bro_node; // delete_fixup은 형제노드와 그 자녀의 색을 통해 진행된다
    if(cur_node == cur_node->parent->left){   // 현재노드가 부모노드의 왼쪽일 때 
      bro_node = cur_node->parent->right;     // 형제노드는 부모노드의 오른쪽이 된다
      if(bro_node->color == RBTREE_RED){      // 형제노드의 색이 붉다면
        bro_node->color = RBTREE_BLACK;       // 형제노드의 색을 BLACK으로 하고
        cur_node->parent->color = RBTREE_RED; // 부모노드의 색을 RED으로 한다 (부모 RED, 현재&형제노드 BLACK)
        rotate_left(t,cur_node->parent);      // 그리고 형제노드를 부모노드자리로 위치하도록 좌회전한다
        bro_node = cur_node->parent->right;   // 현재 노드의 위치도 바뀌었기에(부모노드의 왼쪽으로 레벨 +1이 되었다) 형제노드 갱신
      } // 원래의 형제노드 왼쪽 서브트리는 부모노드의 오른쪽이 되었기에 형제노드가 되어 조작할 수 있게되었다

      // 형제노드의 자식이 모두 검정일 때 형제서브트리의 BlackHeight를 1 낮춰주고(현재노드의 BlackHeight와 같아진다) 부모노드를 기준으로 다음루프로 넘어간다
      if(bro_node->left->color == RBTREE_BLACK && bro_node->right->color == RBTREE_BLACK){
        bro_node->color = RBTREE_RED;
        cur_node = cur_node->parent;
      }
      else {
        if (bro_node->right->color == RBTREE_BLACK) { // 형제노드의 오른쪽자식 노드만 BLACK이라면 == 왼쪽 자식노드만 RED이라면
          bro_node->left->color = RBTREE_BLACK;       // 형제노드의 왼쪽 자식 노드를 BLACK으로 만들고
          bro_node->color = RBTREE_RED;               // 형제노드의 색을 RED로 만든다
          rotate_right(t,bro_node);                   // 왼쪽 자식 노드를 형제 서브트리의 부모로 만든다(왼쪽 자식노드의 자식노드는 항상 BLACK이다)
          bro_node = cur_node->parent->right;         // 형제노드를 갱신해준다
        } // 부모노드가 BLACK인지 RED인지 확실하지 않기에 형제노드의 왼쪽노드가 BLACK노드가 되게 만드는 작업
        bro_node->color = cur_node->parent->color;    // 형제노드의 색은 부모노드의 색이 된다
        cur_node->parent->color = RBTREE_BLACK;       // 부모노드의 색을 BLACK으로 바꾼다
        bro_node->right->color = RBTREE_BLACK;        // 형제노드의 오른쪽 노드 색을 BLACK으로 바꾼다
        rotate_left(t,cur_node->parent);  // 부모노드 기준으로 좌회전하여 BlackHeight를 맞춰준다 (형제트리에서 Black 노드를 하나 가져오는 효과)
        cur_node = t->root; // 루프를 종료한다
      }
    }
    else{ // 현재노드가 부모노드의 오른쪽일 때 
      bro_node = cur_node->parent->left;      // 형제노드는 부모노드의 왼쪽이 된다
      if(bro_node->color == RBTREE_RED){      // 형제노드의 색이 붉다면
        bro_node->color = RBTREE_BLACK;       // 형제노드의 색을 BLACK으로 하고
        cur_node->parent->color = RBTREE_RED; // 부모노드의 색을 RED으로 한다 (부모 RED, 현재&형제노드 BLACK)
        rotate_right(t,cur_node->parent);     // 그리고 형제노드를 부모노드자리로 위치하도록 우회전한다
        bro_node = cur_node->parent->left;    // 현재 노드의 위치도 바뀌었기에(부모노드의 오른쪽이기에 레벨이 +1이 되었다) 형제노드 갱신
      } // 원래의 형제노드 오른쪽 서브트리는 부모노드의 왼쪽이 되었기에 형제노드가 되어 조작할 수 있게되었다

      // 형제노드의 자식이 모두 검정일 때 형제서브트리의 BlackHeight를 1 낮춰주고(현재노드의 BlackHeight와 같아진다) 부모노드를 기준으로 다음루프로 넘어간다
      if(bro_node->right->color == RBTREE_BLACK && bro_node->left->color == RBTREE_BLACK){
        bro_node->color = RBTREE_RED;
        cur_node = cur_node->parent;
      }
      else {
        if (bro_node->left->color == RBTREE_BLACK) { // 형제노드의 왼쪽자식 노드만 BLACK이라면 == 오른쪽 자식노드만 RED이라면
          bro_node->right->color = RBTREE_BLACK;     // 형제노드의 오른쪽 자식 노드를 BLACK으로 만들고
          bro_node->color = RBTREE_RED;              // 형제노드의 색을 RED로 만든다
          rotate_left(t,bro_node);                   // 오른쪽 자식 노드를 형제 서브트리의 부모로 만든다(오른쪽 자식노드의 자식노드는 항상 BLACK이다)
          bro_node = cur_node->parent->left;         // 형제노드를 갱신해준다
        } // 부모노드가 BLACK인지 RED인지 확실하지 않기에 형제노드의 오른쪽노드가 BLACK노드가 되게 만드는 작업
        bro_node->color = cur_node->parent->color;   // 형제노드의 색은 부모노드의 색이 된다
        cur_node->parent->color = RBTREE_BLACK;      // 부모노드의 색을 BLACK으로 바꾼다
        bro_node->left->color = RBTREE_BLACK;        // 형제노드의 왼쪽 노드 색을 BLACK으로 바꾼다
        rotate_right(t,cur_node->parent); // 부모노드 기준으로 우회전하여 BlackHeight를 맞춰준다 (형제트리에서 Black 노드를 하나 가져오는 효과)
        cur_node = t->root; // 루프를 종료한다
      }
    }
  }
  
  // 현재노드가 빨간색일 경우 검정으로 바꿔준다
  cur_node->color = RBTREE_BLACK;
  
  return;
}

// 노드 삭제
int rbtree_erase(rbtree *t, node_t *p) {
  // rbtree를 수정하는데 삭제 노드의 자식노드가 중요하다
  node_t *child_node;
  // 삭제할 노드의 색을 저장한다
  color_t deleted_color = p->color;

  //삭제할 노드의 자식이 한쪽에만 존재할 때 그 자식노드를 부모노드와 연결한다
  if(p->left == t->nil){
    child_node = p->right;
    rbtree_transplant(t,p,child_node);
  }
  else if(p->right == t->nil){
    child_node = p->left;
    rbtree_transplant(t,p,child_node);
  }
  else{ // 삭제할 노드의 자식이 양쪽에 존재할 때 

    // 계승자 노드 탐색 우측자식노드의 가장 왼쪽노드(현재노드보다 큰 가장 작은 노드)
    node_t *min_node = p->right;
    while (min_node->left != t->nil) min_node = min_node->left;
    
    // 이진 트리에서 삭제되는 노드는 계승자노드 이기에 child노드와 삭제되는 색 갱신
    deleted_color = min_node->color;
    child_node = min_node->right;
    
    // 계승자 노드가 삭제되는 노드의 자식노드일때 child노드의 부모는 바뀌지 않음
    if (min_node->parent == p) child_node->parent = min_node;
    else { // 아닐경우 child노드는 계승자 노드가 있던 위치로 바꾸어준다(계승자 노드의 왼쪽은 없다- 가장 왼쪽 노드이기에)
      rbtree_transplant(t,min_node,child_node);
      // 그리고 계승자 노드의 오른쪽을 삭제할 노드의 오른쪽으로 갱신한다
      min_node->right = p->right;
      min_node->right->parent = min_node;
    }
    // 부모 노드에 계승자노드를 붙여준다
    rbtree_transplant(t,p,min_node);

    // 삭제할 노드의 왼쪽을 계승자 노드와 연결한다
    min_node->left = p->left;
    min_node->left->parent = min_node;
    // 계승자 노드는 삭제할 노드의 색을 가진다
    min_node->color = p->color;
  }

  // 삭제할 노드의 색이 BLACK일 때 트리를 조정한다
  if (deleted_color == RBTREE_BLACK) rbtree_delete_fixup(t,child_node);
  
  // 마지막으로 노드의 메모리를 해제한다
  free(p);
  return 0;
}

// 트리의 노드를 중위 순회하여 배열에 삽입
int inorder_travelsal(const rbtree *t, node_t *p, key_t *arr, int index){
  // 왼쪽 자식노드가 nil노드가 아니라면 방문하여 배열에 넣는다 그리고 order를 갱신받는다
  if (p->left != t->nil) index = inorder_travelsal(t,p->left,arr,index);

  // 현재 인덱스를 노드의 key값으로 갱신한다. 그리고 인덱스를 1 올린다
  arr[index] = p->key;
  index++;

  // 오른쪽 자식노드가 nil노드가 아니라면 방문하여 배열에 넣는다 그리고 order를 갱신받는다
  if (p->right != t->nil) index = inorder_travelsal(t,p->right,arr,index);
  
  // 순회를 완료하여 갱신된 index를 반환한다
  return index;
}

// rbtree를 오름차순 배열로 표현  
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  // tree가 비어있을 경우 return 해준다
  if (t->root == t->nil) return 0;

  // 각 key를 맞는 위치에 넣기위해 인덱스로 쓸 정수 i
  int i = 0;
  // 트리의 노드를 중위 순회하여 배열에 삽입
  inorder_travelsal(t, t->root, arr, i);
  
  return 0;
}


void test_find_erase(rbtree *t, const key_t *arr, const size_t n) {
  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_insert(t, arr[i]);
    assert(p != NULL);
  }

  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_find(t, arr[i]);
    printf("arr[%d] = %d\n", i, arr[i]);
    printf("%d",p->key);
    assert(p != NULL);
    assert(p->key == arr[i]);
    rbtree_erase(t, p);
  }

  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_find(t, arr[i]);
    assert(p == NULL);
  }

  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_insert(t, arr[i]);
    assert(p != NULL);
    node_t *q = rbtree_find(t, arr[i]);
    assert(q != NULL);
    assert(q->key == arr[i]);
    assert(p == q);
    rbtree_erase(t, p);
    q = rbtree_find(t, arr[i]);
    assert(q == NULL);
  }
}

void test_find_erase_fixed() {
  const key_t arr[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 24, 36, 990, 25};
  const size_t n = sizeof(arr) / sizeof(arr[0]);
  rbtree *t = new_rbtree();
  assert(t != NULL);

  test_find_erase(t, arr, n);

  delete_rbtree(t);
}

// erase should delete root node
void test_erase_root(const key_t key) {
  rbtree *t = new_rbtree();
  node_t *p = rbtree_insert(t, key);
  assert(p != NULL);
  assert(t->root == p);
  assert(p->key == key);

  rbtree_erase(t, p);
#ifdef SENTINEL
  assert(t->root == t->nil);
#else
  assert(t->root == NULL);
#endif

  delete_rbtree(t);
}

int main()
{
  test_erase_root(8);
  test_find_erase_fixed();

  return 0;
}
