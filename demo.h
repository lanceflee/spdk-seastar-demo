#include "seastar/core/app-template.hh"
#include "seastar/core/future.hh"
#include "seastar/core/distributed.hh"
#include "seastar/core/reactor.hh"
#include "seastar/core/thread.hh"
#include "seastar/core/sharded.hh"

#include <iostream>
#include <exception>
#include "spdk/stdinc.h"
#include "spdk/env.h"
#include "spdk/nvme.h"
#include <string>

extern struct spdk_nvme_ctrlr *g_ctrlr;
class demo_exception : public std::exception{
	virtual	const char* what() const noexcept override {return "Demo Exception";}
};

class io_handler{
	private:
		class demo_context *ctx;
	public:
		void *write_buf, *read_buf;
		bool done;
		const int io_size = 0x1000;
		io_handler(class demo_context *_ctx) : ctx(_ctx) {
			write_buf = spdk_zmalloc(io_size, 0, NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_DMA);
			read_buf = spdk_zmalloc(io_size, 0, NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_DMA);
		}

		static void io_done(void *_c, const struct spdk_nvme_cpl *cpl) {
			class io_handler *c = (class io_handler *)_c;
			c->done = true;
		}
		seastar::future<> handle_io();

};

class demo_context{
    public:
		demo_context(struct spdk_nvme_ns *_ns) : ns(_ns){ 
			this->probed = true; 
		}

		seastar::future<> run() {
			assert(this->probed == true && this->ns != NULL);
        	std::cout << "Demo Starting on ___ " << seastar::this_shard_id() <<"___\n";
			
			this->qpair = spdk_nvme_ctrlr_alloc_io_qpair(g_ctrlr, NULL, 0);
			if(this->qpair == NULL){
				std::cerr << "qpair alloc failed on ___"<< seastar::this_shard_id() <<"___\n";
				return seastar::make_exception_future(demo_exception());
			}

			this->handler = new class io_handler(this);

			std::cout << "Demo running on ___" << seastar::this_shard_id() <<"___\n";
			return handler->handle_io().then([]{
				return seastar::make_ready_future<>();
			});
		}

        seastar::future<> stop(){
			spdk_nvme_ctrlr_free_io_qpair(this->qpair);			
			spdk_free(this->handler->read_buf);
			spdk_free(this->handler->write_buf);
            return seastar::make_ready_future<>();
        }

    public:
        struct spdk_nvme_ns *ns;
		bool probed;
        struct spdk_nvme_qpair *qpair;
		class io_handler *handler;

};


seastar::future<> demo_device_init();
seastar::future<> start_service();