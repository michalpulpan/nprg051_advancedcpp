/*
Stencil1d.hpp
Michal Půlpán
new version
*/

#include <vector>
#include <thread>
#include <math.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <mutex>
#include <condition_variable>


//forward declarations
class Barrier;
template<typename ET>
class circle;
template<typename ET>
class holder;

class Barrier {
public:
    explicit Barrier(std::size_t iCount) : 
      mThreshold(iCount), 
      mCount(iCount), 
      mGeneration(0) {
    }

    void Wait() {
        std::unique_lock<std::mutex> lLock(mMutex);
        auto lGen = mGeneration;
        if (!--mCount) {
            mGeneration++;
            mCount = mThreshold;
            mCond.notify_all();
        } else {
            mCond.wait(lLock, [this, lGen] { return lGen != mGeneration; });
        }
    }

private:
    std::mutex mMutex;
    std::condition_variable mCond;
    std::size_t mThreshold;
    std::size_t mCount;
    std::size_t mGeneration;
};


template<typename ET>
class circle{
public:
    circle(std::size_t size) {
        size_ = size;
        circle_ = std::vector<ET>(size_);
    }

    std::size_t size() const { return size_; }

    void set(std::ptrdiff_t x, const ET& v){ circle_[modulo(x, size_)] = v; }

    ET get(std::ptrdiff_t x) const{ return circle_[modulo(x, size_)]; }

    template<typename SF>
    void run(SF&& sf, std::size_t g, std::size_t thrs = std::thread::hardware_concurrency()){
        //thrs = 4;
        if (thrs > size_)
		    thrs = size_;

        std::vector<std::size_t> partSizes(thrs);
        std::size_t base, toAdd, myG;
        Barrier barrier(thrs);

        base = floor(this->size_/thrs);
        toAdd = size_-(base*thrs);
        for(std::size_t i = 0; i<thrs; i++)
            partSizes[i] = base;
        
        while(toAdd > 0){
            for(std::size_t i = 0; i<thrs; i++){
                partSizes[i] += 1;
                if(!--toAdd)
                    break;
            }
        }

        //myG = g;
/*
        if(myG>base/2)
            myG = floor(base/2);
  */
        myG = std::max(std::min((int)((base - 1) / 2), (int)g),1);
/*

        if(myG < g){
            std::size_t remainder = g % myG;
            std::size_t blocks = ceil(g/myG);

            for(std::size_t i=0;i<blocks-1;i++)
                blocks_.push_back(myG);

            if(remainder == 0){
                blocks_.push_back(myG);
            } else {
                blocks_.push_back(remainder);
            }
            
        } else {
            //no need for dividing into blocks
            blocks_.push_back(myG);
        }*/

        std::size_t circleIndex = 0;
        std::size_t rem = size_ % thrs;
        for(std::size_t i = 0; i <thrs; i++){
            
            if(i<rem){

                holders_.emplace_back(this, &barrier, i, base + 1, myG, circleIndex);
                circleIndex += base +1;

            } else {
                holders_.emplace_back(this, &barrier, i, base, myG, circleIndex);
                circleIndex += base;

            }

            //holders_.push_back(holder<ET>(this, &barrier, i, partSizes[i], myG, circleIndex));
            //circleIndex += partSizes[i];
        }

        for(std::size_t i = 0; i < holders_.size(); i++)
            threads_.emplace_back(std::ref(holders_[i]), std::ref(sf), std::ref(g));

        for(auto&& thread: threads_){
            if(thread.joinable())
                thread.join();
        }

        for(auto&& holder : holders_)
            copyBackToCircle(holder);

        threads_.clear();
        holders_.clear();
        blocks_.clear();
    }


private:
    std::size_t modulo(int a, int b) const{ return a >= 0 ? a % b : ( b - abs ( a%b ) ) % b; }

    void copyBackToCircle(holder<ET>& holder){
        if(holder.usingFirstBuffer_){
            for(std::size_t i=0; i<holder.w_;i++)
                circle_[i+holder.circleIndex_] = holder.buffers_.first[i+holder.g_];
        } else {
            for(std::size_t i=0; i<holder.w_;i++)
                circle_[i+holder.circleIndex_] = holder.buffers_.second[i+holder.g_];
        }
    }

    std::size_t size_;
    std::vector<ET> circle_;
    std::vector<holder<ET>> holders_;
    std::vector<std::thread> threads_;
    std::vector<std::size_t> blocks_;

    friend class holder<ET>;
};

template<typename ET>
class holder{
public:
    holder(circle<ET>* circlePtr, Barrier* share_barrier, std::size_t id, std::size_t w, std::size_t g, std::size_t circleIndex){
        circlePtr_ = circlePtr;
        share_barrier_ = share_barrier;
        id_ = id;
        w_ = w;
        g_ = g;
        circleIndex_ = circleIndex;
        leftNeighbour_ = nullptr;
        rightNeighbour_ = nullptr;

        buffers_.first = std::vector<ET>(g_+w_+g_);
        buffers_.second = std::vector<ET>(g_+w_+g_);
        copyBufferFromCircle();
    }

    template<typename SF>
    void operator()(SF sf, std::size_t g){
        //set neighbour pointers
        std::size_t holdersTotal = circlePtr_->holders_.size();
        leftNeighbour_ = &(circlePtr_->holders_[modulo(id_-1, holdersTotal)]);
        rightNeighbour_ = &(circlePtr_->holders_[modulo(id_+1, holdersTotal)]);

        std::size_t n = g/g_;
        std::size_t remainder = g % g_;

        //std::vector<std::size_t>* blocks = &(circlePtr_->blocks_);
        /*
        for(std::size_t i=0; i<(*blocks).size(); i++){
            for(std::size_t j=0; j<(*blocks)[i]; j++){
                this->calculate(sf);
            }
            share_barrier_->Wait();
            this->synchronize();
            share_barrier_->Wait();
        }
        */
       for(std::size_t block=0; block < n; ++block){
           for(std::size_t gen = 0; gen<g_;++gen){
               calculate(sf);
           }
           share_barrier_->Wait();
           synchronize();
           share_barrier_->Wait();
       }

       for(std::size_t gen = 0; gen<remainder; ++gen){
           calculate(sf);
       }
       share_barrier_->Wait();
       synchronize();
    }

private:
    std::size_t modulo(int a, int b) const{ return a >= 0 ? a % b : ( b - abs ( a%b ) ) % b; }

    template<typename SF>
    void calculate(SF&& sf){       
        
        if(usingFirstBuffer_){
            for(std::size_t i=0; i<=buffers_.first.size()-3; i++)
                buffers_.second[i+1] = sf(buffers_.first[i], buffers_.first[i+1], buffers_.first[i+2]);
            usingFirstBuffer_=false;
        } else {
            for(std::size_t i=0; i<=buffers_.second.size()-3; i++)
                buffers_.first[i+1] = sf(buffers_.second[i], buffers_.second[i+1], buffers_.second[i+2]);
            usingFirstBuffer_=true;
        }


    }

    void synchronize(){
        //right overlap: this->[W+G:W+2G] <-- right->[G:2G]
        for(std::size_t i=0; i<g_; i++){
            setValue(i+w_+g_, rightNeighbour_->getValue(i+g_));
        }
        //left overlap: this->[0:G] <-- left->[W:W+G]
        for(std::size_t i=0; i<g_; i++){
            setValue(i, leftNeighbour_->getValue(i+leftNeighbour_->w_));
        }
    }

    void setValue(std::size_t pos, ET&& value){
        if(usingFirstBuffer_){
            buffers_.first[pos] = value;
        } else {
            buffers_.second[pos] = value;
        }
    }
    
    ET getValue(std::size_t pos){
        if(usingFirstBuffer_){
            return buffers_.first[pos];
        } else {
            return buffers_.second[pos];
        }
    }

    void copyBufferFromCircle(){
        usingFirstBuffer_ = true;
        int start_index = circleIndex_-g_;
        int end_index = circleIndex_+w_+g_;
        int j = 0;

        for(int i=start_index; i<end_index;i++)
            buffers_.first[j++] = circlePtr_->get(i);
        
    }

    circle<ET>* circlePtr_;
    Barrier* share_barrier_;
    std::pair<std::vector<ET>, std::vector<ET>> buffers_;
    std::size_t w_; //defines size of non overlapping/valid cells
    std::size_t g_; //defines size of overlapping/nonvalid cells
    std::size_t id_; //id of thread this instance is initiated in
    std::size_t circleIndex_; //index where circle non overlapping part starts in circle
    holder<ET>* leftNeighbour_;
    holder<ET>* rightNeighbour_;
    bool usingFirstBuffer_ = false;

    friend class circle<ET>;
    friend class holder<ET>;

};