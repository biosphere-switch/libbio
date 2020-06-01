
#pragma once
#include <bio/mem/mem_Memory.hpp>
#include <bio/util/util_Templates.hpp>
#include <bio/util/util_Results.hpp> 

namespace bio::util {

    template<typename T>
    class LinkedList {
        
        private:
            template<typename S>
            struct Node {
                S value;
                Node<S> *prev;
                Node<S> *next;

                constexpr Node(S &val) : value(val), prev(nullptr), next(nullptr) {}

                inline constexpr bool IsHead() {
                    return this->prev == nullptr;
                }

                inline constexpr bool IsTail() {
                    return this->next == nullptr;
                }

                inline constexpr void Push(Node<S> *node) {
                    if(!this->IsTail()) {
                        this->next->prev = node;
                    }
                    node->next = this->next;
                    this->next = node;
                    node->prev = this;
                }

                inline constexpr Node<S> *Pop() {
                    auto ret = this->next;
                    if(!this->IsTail()) {
                        this->next->prev = nullptr;
                        this->next = this->next->next;
                    }
                    return ret;
                }

                inline constexpr Node<S> *PopSelf() {
                    if(!this->IsHead()) {
                        this->prev->next = this->next;
                    }
                    if(!this->IsTail()) {
                        this->next->prev = this->prev;
                    }
                    this->prev = nullptr;
                    this->next = nullptr;
                    return this;
                }

            };

            using ListNode = Node<T>;

            ListNode *head;

            ListNode *GetTailNode() {
                auto cur_node = this->head;
                while(cur_node != nullptr) {
                    if(cur_node->IsTail()) {
                        return cur_node;
                    }
                    cur_node = cur_node->next;
                }
                return nullptr;
            }

            ListNode *GetNodeAtIndex(u32 index) {
                u32 i = 0;
                auto cur_node = this->head;
                while(cur_node != nullptr) {
                    if(i == index) {
                        return cur_node;
                    }
                    cur_node = cur_node->next;
                    i++;
                }
                return nullptr;
            }

        public:
            constexpr LinkedList() : head(nullptr) {}

            inline Result PushBack(T &value) {
                if(this->head == nullptr) {
                    BIO_RES_TRY(mem::New(this->head, value));
                }
                else {
                    ListNode *node;
                    BIO_RES_TRY(mem::New(node, value));

                    auto tail = this->GetTailNode();
                    tail->Push(node);
                }
                return ResultSuccess;
            }
            
            inline Result PushBack(T &&value) {
                if(this->head == nullptr) {
                    BIO_RES_TRY(mem::New(this->head,value));
                }
                else {
                    ListNode *node;
                    BIO_RES_TRY(mem::New(node, value));

                    auto tail = this->GetTailNode();
                    tail->Push(node);
                }
                return ResultSuccess;
            }

            inline Result PushAt(u32 index, T value) {
                auto at_idx = this->GetNodeAtIndex(index);
                BIO_RET_UNLESS(at_idx != nullptr, result::ResultInvalidIndex);

                if(at_idx->IsHead()) {
                    ListNode *node;
                    BIO_RES_TRY(mem::New(node, value));

                    node->next = at_idx;
                    at_idx->prev = node;
                    this->head = node;
                }
                else {
                    ListNode *node;
                    BIO_RES_TRY(mem::New(node, value));

                    auto prev = at_idx->prev;
                    prev->Push(node);
                }
                return ResultSuccess;
            }

            inline void PopBack() {
                auto tail = this->GetTailNode();
                auto node = tail->PopSelf();
                if(node != nullptr) {
                    mem::Delete(node);
                }
            }

            inline void PopAt(u32 index) {
                auto at_idx = this->GetNodeAtIndex(index);
                auto node = at_idx->PopSelf();
                if(node != nullptr) {
                    mem::Delete(node);
                }
            }

            inline constexpr T &GetAt(u32 index) {
                auto at_idx = this->GetNodeAtIndex(index);
                return at_idx->value;
            }

            inline void Clear() {
                auto cur_node = this->GetTailNode();
                while(cur_node != nullptr) {
                    auto prev = cur_node->prev;
                    cur_node->PopSelf();
                    mem::Delete(cur_node);
                    cur_node = prev;
                }
                this->head = nullptr;
            }

            inline constexpr T &Front() {
                return head->value;
            }

            inline constexpr T &Back() {
                auto tail = this->GetTailNode();
                return tail->value;
            }

            inline constexpr bool IsEmpty() {
                return this->head == nullptr;
            }

            inline constexpr bool Any() {
                return !this->IsEmpty();
            }

            inline constexpr u32 GetSize() {
                u32 sz = 0;
                auto cur_node = this->head;
                while(cur_node != nullptr) {
                    cur_node = cur_node->next;
                    sz++;
                }
                return sz;
            }

    };

}