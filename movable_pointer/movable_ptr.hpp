//movable_ptr.hpp
//Michal Pulpan

#pragma once

template<typename T>
class enable_movable_ptr;

template<typename T>
class movable_ptr;


template <typename T>
movable_ptr<T> get_movable(enable_movable_ptr<T>& x){
    T* trg = static_cast<T*>(&x);
    return movable_ptr<T>(trg);
}

template<typename T>
class movable_ptr{
public:
    movable_ptr(): next(nullptr), prev(nullptr), trg(nullptr) {};
    movable_ptr(T* target) noexcept;
    ~movable_ptr();
    movable_ptr(const movable_ptr& second) noexcept; //copy constructor
    movable_ptr(movable_ptr&& second) noexcept; //move constructor

    movable_ptr<T>& operator=(const movable_ptr<T> &second) noexcept; //copy assignemnt
    movable_ptr<T>& operator=(movable_ptr<T>&& second) noexcept; //move assignment

    T& operator*() const {return *(this->trg);}
    T* operator->() const {return this->trg;}
    bool operator!() const { return !get(); }
    bool operator==(const movable_ptr<T>& second) const { return this->trg == second.trg; }
    bool operator!=(const movable_ptr<T>& second) const { return !(this == &second); }
    bool operator==(const T* second) const { return this->trg == second; };
    operator bool() const { return get(); };

    friend movable_ptr<T> get_movable(enable_movable_ptr<T>& x){

    	T* trg = static_cast<T*>(&x);
	return movable_ptr<T>(trg);
    }

    void reset();
    
    void reset(T* x);
    T* get() const;


private:
    //vars
    friend class enable_movable_ptr<T>;
    movable_ptr<T>* next;
    movable_ptr<T>* prev;
    T* trg;

    //functions
    void removeFromLinkedList();
    void copyFunc(const movable_ptr<T> &second);
    bool selfAssignementCheck(const movable_ptr<T> &second);
    void makeMovable(T* target);
    void addTolist(movable_ptr<T>* target);

};

template<typename T>
movable_ptr<T>::movable_ptr(T* target) noexcept{ //create non-empty
    makeMovable(target);
}; 

template<typename T>
movable_ptr<T>::~movable_ptr() { //destructor
        reset();
}; 

template<typename T>
movable_ptr<T>::movable_ptr(const movable_ptr& second) noexcept{
    next = nullptr;
    prev = nullptr;
    trg = nullptr;
    if(selfAssignementCheck(second))
        return;
    this->makeMovable(second.trg);
}

template<typename T>
movable_ptr<T>::movable_ptr(movable_ptr&& second) noexcept{
 
    makeMovable(second.trg);
    second.reset();

}

template<typename T>

void movable_ptr<T>::makeMovable(T* target){

    if(target){

        if (!target->first)	//doesnt have any linked list
        {
            target->first = this;
            this->trg = target;
            this->prev = nullptr;
            this->next = nullptr;
        }
        else  //has linked list
            addTolist(target->first);
    }
}

template<typename T>
bool movable_ptr<T>::selfAssignementCheck(const movable_ptr<T> &second){
    if ((*this) && (second) && (next == second.next && prev == second.prev && trg == second.trg)){
        return true;
    }
    return false;
}

template <typename T>
movable_ptr<T>& movable_ptr<T>::operator=(const movable_ptr<T>& second) noexcept{ //copy assignemnet
    if(selfAssignementCheck(second))
        return *this;
    reset();
    makeMovable(second.trg);
    return *this;
}

template <typename T>
movable_ptr<T>& movable_ptr<T>::operator=(movable_ptr<T>&& second) noexcept{ //move assignemnet
    
    if(selfAssignementCheck(second))
        return *this;

   reset();
   this->makeMovable(second.trg);
   second.reset();
   return *this;
}


template<typename T>
void movable_ptr<T>::reset(){
//removes itself from linked list, set's everything to null and returns itself
    
    if (trg)	
	{//has some target
		if (!(next) && (!prev)) {	
            trg->first = nullptr; //the only movable it's pointing at
        } else {
            removeFromLinkedList(); 
        }
	}
    trg = nullptr;
	next = nullptr;
	prev = nullptr;
    //movable is now removed from linked list, remove all references from it

}

template<typename T>
void movable_ptr<T>::reset(T* x){
    *this = movable_ptr<T>(x);
}

template<typename T>
T* movable_ptr<T>::get() const{
    return this->trg;
}

template<typename T>
void movable_ptr<T>::addTolist(movable_ptr<T>* first){
//ads new item to linked list starting with "first" targeting to "target", returns nothugn    
	movable_ptr<T>* separator = first;	//represents last item of the list
	
	while (separator->next != nullptr && separator->next != first)	//jumps to the end
		separator = separator->next;

	separator->next = this;		//add new item (this) between separator and first
	this->next = first;
	first->prev = this;
	this->prev = separator;
	this->trg = first->trg;
}

template<typename T>
void movable_ptr<T>::removeFromLinkedList(){
//removes itself from linked list, returns nothing

	if (trg->first == this)	// if it's first in linked list 
		trg->first = next;
        
	prev->next = this->next;	//fix linked list
	next->prev = this->prev;

	if (next == next->next)
	{   //it's the only one item in linked list
		next->next = nullptr;
		prev->prev = nullptr;
	}
}

template<typename T>
class enable_movable_ptr{
public:
    enable_movable_ptr(): first(nullptr){};
    ~enable_movable_ptr();//destructor
      
    enable_movable_ptr(const enable_movable_ptr& second) noexcept; //copy constructor
    enable_movable_ptr(enable_movable_ptr&& second) noexcept; //move constructor

    enable_movable_ptr<T>& operator=(const enable_movable_ptr& second) noexcept; //copy assignemnt
    enable_movable_ptr<T>& operator=(enable_movable_ptr&& second) noexcept; //move assignment
    bool operator==(const T* second) const { return (this->first == second->first); }

private:
    friend class movable_ptr<T>;
    movable_ptr<T>* first;
    void reset();
    void moveFunc( movable_ptr<T>* first);
    bool selfAssignementCheck(const enable_movable_ptr<T>& second);

};

template<typename T>
enable_movable_ptr<T>::~enable_movable_ptr(){//destructor
       reset();
};

template<typename T>
enable_movable_ptr<T>::enable_movable_ptr(enable_movable_ptr&& second) noexcept{
    if(selfAssignementCheck(second))
        return;
    first = second.first;
    moveFunc(second.first);
    second.first = nullptr;
}

template<typename T>
enable_movable_ptr<T>::enable_movable_ptr(const enable_movable_ptr& second) noexcept{
    if(selfAssignementCheck(second))
        return;
    this->first = nullptr;
}

template<typename T>
bool enable_movable_ptr<T>::selfAssignementCheck(const enable_movable_ptr<T>& second){
    if(this==(&second))
        return true;
    return false;
}

template<typename T>
void enable_movable_ptr<T>::reset(){

	if (!(first))	//no pointers to remove
		return;
	movable_ptr<T>* iter = first->next;
	movable_ptr<T>* separator = first;
	separator->reset();
	while (iter)	//remove whole linked list
	{
		separator = iter;
		iter = iter->next;
		separator->reset();
	}
	first = nullptr;

}

template<typename T>
void enable_movable_ptr<T>::moveFunc( movable_ptr<T>* first){

    if(first){ //if linked-list exists, set all targets to this
        movable_ptr<T>* current = first;
        current->trg = static_cast<T*>(this);
        current = current->next;
        while (current != nullptr&&current != first)	//set all movable pointers their new target
        {
            current->trg = static_cast<T*>(this);
            current = current->next;
        }
    }
}

template<typename T>
enable_movable_ptr<T>& enable_movable_ptr<T>::operator=(const enable_movable_ptr<T>& second) noexcept{ //copy assignemnt 
    if(selfAssignementCheck(second))
        return *this;
    reset();
    return *this;

}

template<typename T>
enable_movable_ptr<T>& enable_movable_ptr<T>::operator=(enable_movable_ptr<T>&& second) noexcept{ //move assignemnt 

    if(selfAssignementCheck(second))
        return *this;
    reset(); //destroy current linked list (remove targets)
    first = second.first;
    moveFunc(second.first);
    second.first = nullptr;
    return *this;    

}

