#include "demo.h"


seastar::sharded<demo_context> ctx;
static std::string g_device_name = "trtype:PCIe traddr:0000:01:00.0";


seastar::future<> io_handler::handle_io()
{
	snprintf((char *)write_buf, io_size, "%s", "Hello world!\n");
	this->done = false;
	spdk_nvme_ns_cmd_write(this->ctx->ns, this->ctx->qpair, write_buf, 0, io_size / 512, this->io_done, this, 0);
	
	//seastar::do_until: Polling cqueue
	return seastar::do_until([this] { return done; }, [this] {
		spdk_nvme_qpair_process_completions(this->ctx->qpair, 0);
		return seastar::make_ready_future<>();
		}).then([this] {
			done = false;
			spdk_nvme_ns_cmd_read(this->ctx->ns, this->ctx->qpair, read_buf, 0, io_size / 512, this->io_done, this, 0);
			
			return seastar::do_until([this] { return done; }, [this] {
					spdk_nvme_qpair_process_completions(this->ctx->qpair, 0);
					return seastar::make_ready_future<>();
					}).then([this] { 
						if (memcmp(read_buf, write_buf, io_size) == 0) {
						printf("%s", (char *)read_buf);
						} else {
						printf("Data miscompare\n");
						}
						});
			});
}

//SPDK: NVMe Devices Probe
struct spdk_nvme_ctrlr *g_ctrlr;

static bool  probe_cb(void *cb_ctx, 
                      const struct spdk_nvme_transport_id *trid, 
                      struct spdk_nvme_ctrlr_opts *opts)
{
	opts->no_shn_notification = true;
	return true;
}

static void attach_cb(void *cb_ctx, 
		              const struct spdk_nvme_transport_id *trid, 
                      struct spdk_nvme_ctrlr *ctrlr,		
                      const struct spdk_nvme_ctrlr_opts *opts)
{
	std::cout << "Attached to " << trid->traddr << "\n";
	g_ctrlr = ctrlr;
}

static int
add_trid(struct spdk_nvme_transport_id *trid, const char *trid_str)
{

	trid->trtype = SPDK_NVME_TRANSPORT_PCIE;
	snprintf(trid->subnqn, sizeof(trid->subnqn), "%s", SPDK_NVMF_DISCOVERY_NQN);


	if (spdk_nvme_transport_id_parse(trid, trid_str) != 0) {
		std::cerr << "Failed to Probe NVMe Devices:" <<trid_str<<"\n";
		return 1;
	}

	spdk_nvme_transport_id_populate_trstring(trid,
			spdk_nvme_transport_id_trtype_str(trid->trtype));

	return 0;
}

seastar::future<> demo_device_init(){
	struct spdk_nvme_transport_id trid;
	struct spdk_env_opts opts;

    //Replace traddr and pay great attention to Intel VMD devices
	
	spdk_env_opts_init(&opts);
	opts.name = "demo";
	if(spdk_env_init(&opts)<0){
		std::cerr << "Failed to init env" <<"\n";
		return seastar::make_exception_future<>(demo_exception());
	}
	memset(&trid, 0, sizeof(trid));
	add_trid(&trid,g_device_name);

	//Local Device
	//trid.trtype = SPDK_NVME_TRANSPORT_PCIE;
	if (spdk_nvme_probe(&trid, NULL, probe_cb, attach_cb, NULL) != 0) {
		return seastar::make_exception_future<>(demo_exception());
	}
	return seastar::make_ready_future<>();
}

seastar::future<> start_service(){
	struct spdk_nvme_ns *_ns;

	_ns = spdk_nvme_ctrlr_get_ns(g_ctrlr,1);

	return ctx.start(_ns).then([]{
		return ctx.invoke_on_all([](demo_context& local_ctx){
			return local_ctx.run();
		});
	}).then([]{
		return ctx.stop();
	});
}

int main(int argc, char** argv) {

	seastar::app_template app;
	namespace bpo = boost::program_options;
    app.add_options()
        ("device",bpo::value<seastar::sstring>()->default_value("trtype:PCIe traddr:0000:01:00.0"),"device addr")
        ;

	try {
		app.run(argc, argv, [&app]{
			auto& args = app.configuration();
            std::string device = args["device"].as<seastar::sstring>().c_str();
			if(device){
				g_device_name = device;
				std::cout<<"Using device addr:"<<g_device_name<<std::endl;
			}
			return demo_device_init().then([]{return start_service();});
		});
		
		spdk_nvme_detach(g_ctrlr);
	} catch (...) {

		std::cerr << "Failed to start application: "

			<< std::current_exception() << "\n";

		return 1;

	}

	return 0;

}
