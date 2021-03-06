// XMPI.h
// created by Kuangdai on 28-Jun-2016 
// mpi interfaces

#pragma once 
#include <iostream>
#include "eigenc.h"
#include <map>

#ifndef _SERIAL_BUILD
    #include "mpi.h"
#else
    #define MPI_Request int
    #define MPI_Datatype int
    #define MPI_CHAR 1
    #define MPI_INT 2
    #define MPI_FLOAT 3
    #define MPI_DOUBLE 4
#endif

class XMPI {
public:
    // initialize and finalize
    static void initialize(int argc, char *argv[]);
    static void finalize();
    
    // print exception
    static void printException(const std::exception &e);
    
    // properties
    static int nproc() {
        #ifndef _SERIAL_BUILD
            int world_size;
            MPI_Comm_size(MPI_COMM_WORLD, &world_size);
            return world_size;
        #else
            return 1;
        #endif
    };
    
    static int rank() {
        #ifndef _SERIAL_BUILD
            int world_rank;
            MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
            return world_rank;
        #else
            return 0;
        #endif
    };
    
    static bool root() {return rank() == 0;};
    
    // barrier
    static void barrier() {
        #ifndef _SERIAL_BUILD
            MPI_Barrier(MPI_COMM_WORLD);
        #endif
    };
    
    // abort
    static void abort(int err = 0) {
        #ifndef _SERIAL_BUILD
            MPI_Abort(MPI_COMM_WORLD, err);
        #else
            exit(err);
        #endif
    };
    
    ////////////////////////////// broadcast //////////////////////////////
    // array
    static void bcast(int *buffer, int size);
    static void bcast(double *buffer, int size);
    static void bcast(float *buffer, int size);
    static void bcast(std::complex<float> *buffer, int size);
    static void bcast(std::complex<double> *buffer, int size);
    static void bcast(char *buffer, int size);
    template<typename Type>
    static void bcast_alloc(Type *&buffer, int size) {
        #ifndef _SERIAL_BUILD
            bcast(size);
            if (!root()) {
                if (buffer != 0) delete [] buffer;
                buffer = new Type[size];
            }
            bcast(buffer, size);
        #endif
    };
    
    // single
    static void bcast(int &buffer);
    static void bcast(double &buffer);
    static void bcast(float &buffer);
    static void bcast(std::string &str);
    
    // Eigen::Matrix
    template<typename Type>
    static void bcastEigen(Type &buffer) {
        #ifndef _SERIAL_BUILD
            int dim[2];
            if (root()) {dim[0] = buffer.rows(); dim[1] = buffer.cols();}
            bcast(dim, 2);
            if (!root()) buffer = Type::Zero(dim[0], dim[1]);
            bcast(buffer.data(), buffer.size());
        #endif
    };
    
    // std::vector
    template<typename Type>
    static void bcast(std::vector<Type> &buffer) {
        #ifndef _SERIAL_BUILD
            int size = 0;
            if (root()) size = buffer.size();
            bcast(size);
            if (!root()) buffer.resize(size);
            bcast(buffer.data(), size);
        #endif
    }
    
    // special case
    static void bcast(std::vector<std::string> &buffer);
    
    ////////////////////////////// isend/irecv ////////////////////////////// 
    // isend, only for Eigen::Matrix
    template<typename EigenMat>
    static void isendDouble(int dest, const EigenMat &buffer, MPI_Request &request) {
        #ifndef _SERIAL_BUILD
            MPI_Isend(buffer.data(), buffer.size(), 
                MPI_DOUBLE, dest, dest, MPI_COMM_WORLD, &request);
        #endif
    };
    
    // irecv, only for Eigen::Matrix
    template<typename EigenMat>
    static void irecvDouble(int source, EigenMat &buffer, MPI_Request &request) {
        #ifndef _SERIAL_BUILD
            MPI_Irecv(buffer.data(), buffer.size(), 
                MPI_DOUBLE, source, rank(), MPI_COMM_WORLD, &request);
        #endif
    };
    
    // isend, only for Eigen::Matrix
    template<typename EigenMat>
    static void isendComplex(int dest, const EigenMat &buffer, MPI_Request &request) {
        #ifndef _SERIAL_BUILD
            #ifdef _USE_DOUBLE
                MPI_Isend(buffer.data(), buffer.size(), 
                    MPI_C_DOUBLE_COMPLEX, dest, dest, MPI_COMM_WORLD, &request);
            #else
                MPI_Isend(buffer.data(), buffer.size(), 
                    MPI_C_FLOAT_COMPLEX, dest, dest, MPI_COMM_WORLD, &request);
            #endif
        #endif
    };
    
    // irecv, only for Eigen::Matrix
    template<typename EigenMat>
    static void irecvComplex(int source, EigenMat &buffer, MPI_Request &request) {
        #ifndef _SERIAL_BUILD
            #ifdef _USE_DOUBLE
                MPI_Irecv(buffer.data(), buffer.size(), 
                    MPI_C_DOUBLE_COMPLEX, source, rank(), MPI_COMM_WORLD, &request);
            #else
                MPI_Irecv(buffer.data(), buffer.size(), 
                    MPI_C_FLOAT_COMPLEX, source, rank(), MPI_COMM_WORLD, &request);
            #endif
        #endif
    };
    
    // wait_all
    static void wait_all(int count, MPI_Request array_of_requests[]) {
        #ifndef _SERIAL_BUILD
            MPI_Waitall(count, array_of_requests, MPI_STATUSES_IGNORE);
        #endif
    };
    
    ////////////////////////////// gather ////////////////////////////// 
    // string
    static void gather(int buf, std::vector<int> &all_buf, bool all);
    static void gather(const std::string &buf, std::vector<std::string> &all_buf, bool all);
    static void gather(const std::vector<std::string> &buf, 
        std::vector<std::vector<std::string>> &all_buf, bool all);
        
    template<typename Type>
    static void gather(const std::vector<Type> &buf, 
        std::vector<std::vector<Type>> &all_buf, MPI_Datatype mpitype, 
        bool all) {
        
        #ifndef _SERIAL_BUILD
            // size
            int size = buf.size();
            std::vector<int> all_size;
            gather(size, all_size, all);
            int total_size = 0;
            for (auto &n : all_size) total_size += n;
            int nproc = XMPI::nproc();
            std::vector<int> disp(nproc, 0);
            if (all || root()) {
                for (int i = 1; i < nproc; i++) 
                    for (int j = 0; j < i; j++) 
                        disp[i] += all_size[j];
            }

            std::vector<Type> allBuf_flat;
            if (all || root()) allBuf_flat.resize(total_size);
            if (all) {
                MPI_Allgatherv(buf.data(), size, mpitype, allBuf_flat.data(), all_size.data(), disp.data(), mpitype, MPI_COMM_WORLD);
            } else {
                MPI_Gatherv(buf.data(), size, mpitype, allBuf_flat.data(), all_size.data(), disp.data(), mpitype, 0, MPI_COMM_WORLD);
            }
            if (all || root()) {
                all_buf.clear();
                int pos = 0;
                for (int i = 0; i < nproc; i++) {
                    std::vector<Type> sub;
                    for (int j = 0; j < all_size[i]; j++) sub.push_back(allBuf_flat[pos++]);
                    all_buf.push_back(sub);
                }
            }
            
        #else
            all_buf.clear();
            all_buf.push_back(buf);
        #endif
    }
    
    template<typename Type>
    static void gather(const std::map<std::string, Type> &buf, 
        std::vector<std::map<std::string, Type>> &all_buf, 
        MPI_Datatype mpitype, bool all) {
        
        std::vector<std::string> keys;
        std::vector<Type> vals;
        for (auto it = buf.begin(); it != buf.end(); it++) {
            keys.push_back(it->first);
            vals.push_back(it->second);
        }
        
        std::vector<std::vector<std::string>> all_keys;
        std::vector<std::vector<Type>> all_vals;
        gather(keys, all_keys, all);
        gather(vals, all_vals, mpitype, all);
        
        if (all || root()) {
            all_buf.clear();
            for (int i = 0; i < all_keys.size(); i++) {
                std::map<std::string, Type> sub;
                for (int j = 0; j < all_keys[i].size(); j++)
                    sub.insert(std::pair<std::string, Type>(all_keys[i][j], all_vals[i][j]));
                all_buf.push_back(sub);
            }
        }
    }
    
    ////////////////////////////// reduce //////////////////////////////
    // simple
    static int min(const int &value);
    static double min(const double &value);
    static int max(const int &value);
    static double max(const double &value);
    static int sum(const int &value);
    static double sum(const double &value);
    
    // sum std::vector
    static void sumVector(std::vector<double> &value);
    
    // sum Eigen::Matrix
    template<typename Type>
    static void sumEigenDouble(Type &value) {
        #ifndef _SERIAL_BUILD
            Type total(value);
            MPI_Allreduce(value.data(), total.data(), value.size(), MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
            value = total;
        #endif
    };
    
    template<typename Type>
    static void sumEigenInt(Type &value) {
        #ifndef _SERIAL_BUILD
            Type total(value);
            MPI_Allreduce(value.data(), total.data(), value.size(), MPI_INT, MPI_SUM, MPI_COMM_WORLD);
            value = total;
        #endif
    };
        
    ////////////////////////////// stream on root //////////////////////////////
    struct root_cout {
        template<typename Type>
        root_cout &operator<<(const Type &val) {
            if (rank() == mProc) std::cout << val;
            return *this;
        };
        void resetp() {mProc = 0;};
        void setp(int proc) {
            if (proc < nproc()) mProc = proc;
            else resetp();
        };
        int mProc = 0;
    };
    static root_cout cout;
    static std::string endl;
    
    ////////////////////////////// dir utils //////////////////////////////
    static bool dirExists(const std::string &path);
    static void mkdir(const std::string &path);
    
};

// message info
struct MessagingInfo {
    // number of procs to communicate with
    int mNProcComm;
    // procs to communicate with
    std::vector<int> mIProcComm;
    // number of local points to be communicated for each proc
    std::vector<int> mNLocalPoints;
    // indecies of local points to be communicated for each proc
    std::vector<std::vector<int>> mILocalPoints;
    // mpi requests
    std::vector<MPI_Request> mReqSend;
    std::vector<MPI_Request> mReqRecv;
};

// message buffer for solver
struct MessagingBuffer {
    std::vector<CColX> mBufferSend;
    std::vector<CColX> mBufferRecv;
};


