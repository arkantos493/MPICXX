#ifndef MPICXX_INFO_HPP
#define MPICXX_INFO_HPP

#include <mpi.h>
#include <optional>
#include <string>

namespace mpicxx {

    class info {
    public:
        // constructors
        info() {
            MPI_Info_create(&info_);
        }
        info(const info& other) {
            MPI_Info_dup(other.info_, &info_);
        }
        info(info&&) = delete; // TODO: test

        // destructor
        ~info() {
            MPI_Info_free(&info_);
        }

        // assignment operator
        info& operator=(const info& rhs) {
            if (this != &rhs) {
                MPI_Info_free(&info_);
                MPI_Info_dup(rhs.info_, &info_);
            }
            return *this;
        }
        info& operator=(info&&) = delete; // TODO: test

        // TODO: const char*??
        std::optional<std::string> get_value(std::string&& key) {
            // TODO: assert key.size() < MPI_MAX_INFO_KEY

            std::string value(key.size(), ' ');
            int flag;
            // TODO: size
            MPI_Info_get(info_, key.c_str(), key.size(), const_cast<char*>(value.c_str()), &flag);

            if (flag != 0) {
                return std::make_optional(value);
            } else {
                return std::nullopt;
            }
        }

        bool delete_key(std::string&& key) {
            int error_code = MPI_Info_delete(info_, key.c_str());
            return error_code != MPI_ERR_INFO_NOKEY;
        }

        int get_nkeys() {
            int nkeys;
            MPI_Info_get_nkeys(info_, &nkeys);
            return nkeys;
        }

        std::string get_nthkey(const int n) {
            char key[MPI_MAX_INFO_KEY];
            MPI_Info_get_nthkey(info_, n, key);
            return std::string(key, std::char_traits<char>::length(key));
        }

    private:
        MPI_Info info_;
    };

}

#endif //MPICXX_INFO_HPP
