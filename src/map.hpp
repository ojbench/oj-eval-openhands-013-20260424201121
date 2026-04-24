/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   typedef pair<const Key, T> value_type;

  private:
    struct Node {
        value_type *data;
        Node *left, *right, *parent;
        int height;

        Node(const value_type &v) : left(nullptr), right(nullptr), parent(nullptr), height(1) {
            data = (value_type*) ::operator new(sizeof(value_type));
            new(data) value_type(v);
        }
        Node() : data(nullptr), left(nullptr), right(nullptr), parent(nullptr), height(0) {}
        ~Node() {
            if (data) {
                data->~value_type();
                ::operator delete(data);
            }
        }
    };

    Node *header;
    size_t _size;
    Compare compare;

    int getHeight(Node *p) const { return p ? p->height : 0; }
    void updateHeight(Node *p) {
        if (p) {
            int hl = getHeight(p->left);
            int hr = getHeight(p->right);
            p->height = (hl > hr ? hl : hr) + 1;
        }
    }
    int getBalance(Node *p) const { return p ? getHeight(p->left) - getHeight(p->right) : 0; }

    void rotateR(Node *&p) {
        Node *l = p->left;
        p->left = l->right;
        if (l->right) l->right->parent = p;
        l->parent = p->parent;
        if (p->parent->left == p) p->parent->left = l;
        else p->parent->right = l;
        l->right = p;
        p->parent = l;
        updateHeight(p);
        updateHeight(l);
        p = l;
    }

    void rotateL(Node *&p) {
        Node *r = p->right;
        p->right = r->left;
        if (r->left) r->left->parent = p;
        r->parent = p->parent;
        if (p->parent->left == p) p->parent->left = r;
        else p->parent->right = r;
        r->left = p;
        p->parent = r;
        updateHeight(p);
        updateHeight(r);
        p = r;
    }

    void balance(Node *&p) {
        if (!p) return;
        updateHeight(p);
        int b = getBalance(p);
        if (b > 1) {
            if (getBalance(p->left) < 0) rotateL(p->left);
            rotateR(p);
        } else if (b < -1) {
            if (getBalance(p->right) > 0) rotateR(p->right);
            rotateL(p);
        }
    }

    Node* findNode(const Key &key) const {
        Node *p = (header->parent == header ? nullptr : header->parent);
        while (p) {
            if (compare(key, p->data->first)) p = p->left;
            else if (compare(p->data->first, key)) p = p->right;
            else return p;
        }
        return nullptr;
    }

    void clear(Node *&p) {
        if (!p || p == header) return;
        clear(p->left);
        clear(p->right);
        delete p;
        p = nullptr;
    }

    Node* copy(Node *p, Node *pa) {
        if (!p || p == pa) return nullptr; // pa is header
        Node *res = new Node(*(p->data));
        res->parent = pa;
        res->height = p->height;
        res->left = copy(p->left, res);
        res->right = copy(p->right, res);
        return res;
    }

    Node* getMostLeft() const {
        Node *p = (header->parent == header ? nullptr : header->parent);
        if (!p) return header;
        while (p->left) p = p->left;
        return p;
    }

    Node* getMostRight() const {
        Node *p = (header->parent == header ? nullptr : header->parent);
        if (!p) return header;
        while (p->right) p = p->right;
        return p;
    }

  public:
   class const_iterator;
   class iterator {
      friend class map;
      private:
        Node *ptr;
        const map *m;
      public:
       iterator(Node *p = nullptr, const map *map_ptr = nullptr) : ptr(p), m(map_ptr) {}
       iterator(const iterator &other) : ptr(other.ptr), m(other.m) {}

       iterator operator++(int) {
           iterator tmp = *this;
           ++(*this);
           return tmp;
       }
       iterator &operator++() {
           if (ptr == m->header) throw invalid_iterator();
           if (ptr->right) {
               ptr = ptr->right;
               while (ptr->left) ptr = ptr->left;
           } else {
               Node *pa = ptr->parent;
               while (pa != m->header && ptr == pa->right) {
                   ptr = pa;
                   pa = pa->parent;
               }
               ptr = pa;
           }
           return *this;
       }
       iterator operator--(int) {
           iterator tmp = *this;
           --(*this);
           return tmp;
       }
       iterator &operator--() {
           if (ptr == m->getMostLeft()) throw invalid_iterator();
           if (ptr == m->header) {
               ptr = m->getMostRight();
           } else if (ptr->left) {
               ptr = ptr->left;
               while (ptr->right) ptr = ptr->right;
           } else {
               Node *pa = ptr->parent;
               while (pa != m->header && ptr == pa->left) {
                   ptr = pa;
                   pa = pa->parent;
               }
               if (pa == m->header) throw invalid_iterator();
               ptr = pa;
           }
           return *this;
       }
       value_type &operator*() const {
           if (ptr == m->header || !ptr) throw invalid_iterator();
           return *(ptr->data);
       }
       bool operator==(const iterator &rhs) const { return ptr == rhs.ptr && m == rhs.m; }
       bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr && m == rhs.m; }
       bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
       bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
       value_type *operator->() const noexcept {
           if (ptr == m->header || !ptr) return nullptr;
           return ptr->data;
       }
   };
   class const_iterator {
      friend class map;
      private:
        Node *ptr;
        const map *m;
      public:
       const_iterator(Node *p = nullptr, const map *map_ptr = nullptr) : ptr(p), m(map_ptr) {}
       const_iterator(const const_iterator &other) : ptr(other.ptr), m(other.m) {}
       const_iterator(const iterator &other) : ptr(other.ptr), m(other.m) {}

       const_iterator operator++(int) {
           const_iterator tmp = *this;
           ++(*this);
           return tmp;
       }
       const_iterator &operator++() {
           if (ptr == m->header) throw invalid_iterator();
           if (ptr->right) {
               ptr = ptr->right;
               while (ptr->left) ptr = ptr->left;
           } else {
               Node *pa = ptr->parent;
               while (pa != m->header && ptr == pa->right) {
                   ptr = pa;
                   pa = pa->parent;
               }
               ptr = pa;
           }
           return *this;
       }
       const_iterator operator--(int) {
           const_iterator tmp = *this;
           --(*this);
           return tmp;
       }
       const_iterator &operator--() {
           if (ptr == m->getMostLeft()) throw invalid_iterator();
           if (ptr == m->header) {
               ptr = m->getMostRight();
           } else if (ptr->left) {
               ptr = ptr->left;
               while (ptr->right) ptr = ptr->right;
           } else {
               Node *pa = ptr->parent;
               while (pa != m->header && ptr == pa->left) {
                   ptr = pa;
                   pa = pa->parent;
               }
               if (pa == m->header) throw invalid_iterator();
               ptr = pa;
           }
           return *this;
       }
       const value_type &operator*() const {
           if (ptr == m->header || !ptr) throw invalid_iterator();
           return *(ptr->data);
       }
       bool operator==(const iterator &rhs) const { return ptr == rhs.ptr && m == rhs.m; }
       bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr && m == rhs.m; }
       bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
       bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
       const value_type *operator->() const noexcept {
           if (ptr == m->header || !ptr) return nullptr;
           return ptr->data;
       }
   };

   map() : _size(0) {
       header = new Node();
       header->parent = header->left = header->right = header;
   }

   map(const map &other) : _size(other._size), compare(other.compare) {
       header = new Node();
       header->left = header->right = header;
       header->parent = copy(other.header->parent, header);
       if (!header->parent) header->parent = header;
   }

   map &operator=(const map &other) {
       if (this == &other) return *this;
       clear();
       delete header;
       _size = other._size;
       compare = other.compare;
       header = new Node();
       header->left = header->right = header;
       header->parent = copy(other.header->parent, header);
       if (!header->parent) header->parent = header;
       return *this;
   }

   ~map() {
       clear();
       delete header;
   }

   T &at(const Key &key) {
       Node *p = findNode(key);
       if (!p) throw index_out_of_bound();
       return p->data->second;
   }

   const T &at(const Key &key) const {
       Node *p = findNode(key);
       if (!p) throw index_out_of_bound();
       return p->data->second;
   }

   T &operator[](const Key &key) {
       Node *p = findNode(key);
       if (p) return p->data->second;
       T default_value = T();
       return insert(pair<const Key, T>(key, default_value)).first.ptr->data->second;
   }

   const T &operator[](const Key &key) const {
       Node *p = findNode(key);
       if (!p) throw index_out_of_bound();
       return p->data->second;
   }

   iterator begin() { return iterator(getMostLeft(), this); }
   const_iterator cbegin() const { return const_iterator(getMostLeft(), this); }
   iterator end() { return iterator(header, this); }
   const_iterator cend() const { return const_iterator(header, this); }
   bool empty() const { return _size == 0; }
   size_t size() const { return _size; }

   void clear() {
       if (header->parent != header) {
           clear(header->parent);
           header->parent = header;
       }
       _size = 0;
   }

   pair<iterator, bool> insert(const value_type &value) {
       Node *p = (header->parent == header ? nullptr : header->parent);
       Node *pa = header;
       while (p) {
           if (compare(value.first, p->data->first)) {
               pa = p;
               p = p->left;
           } else if (compare(p->data->first, value.first)) {
               pa = p;
               p = p->right;
           } else {
               return pair<iterator, bool>(iterator(p, this), false);
           }
       }
       _size++;
       Node *newNode = new Node(value);
       newNode->parent = pa;
       if (pa == header) header->parent = newNode;
       else if (compare(value.first, pa->data->first)) pa->left = newNode;
       else pa->right = newNode;
       
       p = newNode;
       while (p != header) {
           updateHeight(p);
           Node *parent = p->parent;
           int b = getBalance(p);
           if (b > 1 || b < -1) {
               Node *&ref = (parent == header ? header->parent : (parent->left == p ? parent->left : parent->right));
               balance(ref);
               break;
           }
           p = parent;
       }
       return pair<iterator, bool>(iterator(newNode, this), true);
   }
   void erase(iterator pos) {
       if (pos.m != this || pos.ptr == header || !pos.ptr) throw invalid_iterator();
       _size--;
       Node *p = pos.ptr;
       if (p->left && p->right) {
           Node *tmp = p->right;
           while (tmp->left) tmp = tmp->left;
           p->data->~value_type();
           new(p->data) value_type(*(tmp->data));
           p = tmp;
       }
       Node *pa = p->parent;
       Node *child = (p->left ? p->left : p->right);
       if (pa == header) header->parent = child;
       else if (pa->left == p) pa->left = child;
       else pa->right = child;
       if (child) child->parent = pa;
       delete p;
       
       p = pa;
       while (p != header) {
           updateHeight(p);
           Node *parent = p->parent;
           Node *&ref = (parent == header ? header->parent : (parent->left == p ? parent->left : parent->right));
           balance(ref);
           p = parent;
       }
   }

   size_t count(const Key &key) const {
       return findNode(key) ? 1 : 0;
   }

   iterator find(const Key &key) {
       Node *p = findNode(key);
       return p ? iterator(p, this) : end();
   }

   const_iterator find(const Key &key) const {
       Node *p = findNode(key);
       return p ? const_iterator(p, this) : cend();
   }
};

}

#endif