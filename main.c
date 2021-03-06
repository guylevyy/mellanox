#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vl.h>
#include "types.h"
#include "resources.h"
#include "test.h"
#include "infiniband/verbs.h"

struct config_t config = {
	.hca_type = "mlx5_0",
	.ip = "127.0.0.1",
	.num_of_iter = 8,
	.is_daemon = 0,
	.wait = 0,
	.tcp = 17500,
	.qp_type = IBV_QPT_RC,
	.opcode = IBV_WR_SEND,
	.msg_sz = 8,
	.ring_depth = DEF_RING_DEPTH,
	.batch_size = DEF_BATCH_SIZE,
	.send_method = METHOD_OLD,
	.use_inl = 0,
	.num_sge = DEF_NUM_SGE,
	.ext_atomic = 0,
};

struct VL_usage_descriptor_t usage_descriptor[] = {
	{
		'h', "help", "",
		"Print this message and exit",
#define HELP_CMD_CASE				0
		HELP_CMD_CASE
	},

	{
		'I', "use_inl", "",
		"Use inline data to post",
#define INL_CMD_CASE				1
		INL_CMD_CASE
	},

	{
		'r', "ring_sz", "RING_SZ",
		"the ring to be used on the TX and RX queues (Default 64)",
#define RING_CMD_CASE				2
		RING_CMD_CASE
	},

	{
		' ', "num_sge", "NUM_SGE",
		"Number of SGEs to be used (Default 1)",
#define SGE_CMD_CASE				3
		SGE_CMD_CASE
	},

	{
		'o', "opcode", "OPCODE",
		"Message opcode (SEND (default), SEND_IMM, SEND_INV, WRITE, WRITE_IMM, READ, BIND, LOCAL_INV,"
		"\n\tATOMIC_FA, ATOMIC_CS, EXT_ATOMIC_FA, EXT_ATOMIC_CS)",
#define OP_CMD_CASE				4
		OP_CMD_CASE
	},

	{
		'm', "method", "METHOD",
		"Post send method [OLD, NEW, MIX] (default: OLD)",
#define SND_MTD_CMD_CASE			5
		SND_MTD_CMD_CASE
	},

	{
		'i', "iteration", "ITERATION",
		"The number of iteration for this test (Default 8)",
#define NUM_OF_ITER_CMD_CASE			6
		NUM_OF_ITER_CMD_CASE
	},

	{
		' ', "mac", "MAC_ADDR",
		"Local mac to use for Raw packet transport",
#define MAC_CMD_CASE				7
		MAC_CMD_CASE
	},

	{
		'd', "device", "HOST_ID",
		"HCA to use - Default mlx5_0",
#define HOST_CMD_CASE				8
		HOST_CMD_CASE
	},

	{
		'w', "wait", "",
		"Wait before exit",
#define WAIT_CMD_CASE				9
		WAIT_CMD_CASE
	},

	{
		' ', "ip", "IP_ADDR",
		"The ip of the srq_test server. Default value: current machine.",
#define IP_CMD_CASE				10
		IP_CMD_CASE
	},

	{
		' ', "daemon", "",
		"Run as a server.",
#define DAEMON_CMD_CASE				12
		DAEMON_CMD_CASE
	},

	{
		' ', "tcp", "TCP",
		"TCP port to use",
#define TCP_CMD_CASE				14
		TCP_CMD_CASE
	},

	{
		'b', "batch", "BATCH",
		"WRs list size to post",
#define BATCH_CMD_CASE				15
		BATCH_CMD_CASE
	},

	{
		't', "qp_type", "QP_TYPE",
		"Enforce QPs type [RC/DC/UD/RAW/XRC (Default: RC)]",
#define QP_TYPE_CMD_CASE			16
		QP_TYPE_CMD_CASE
	}

};

/***********************************
* Function: bool_to_str.
************************************/
/* 0 = FALSE. */
const char *bool_to_str(
	IN		int var)
{
	if (var)
		return "YES";

	return "NO";
}

/***********************************
* Function: print_config.
************************************/
static void print_config(void)
{
	VL_MISC_TRACE((" ---------------------- config data  ---------------"));

	VL_MISC_TRACE((" Test side                      : %s", ((config.is_daemon) ? "Server" : "Client")));
	if (!config.is_daemon)
		VL_MISC_TRACE((" IP                             : %s", config.ip));
	VL_MISC_TRACE((" TCPort                         : %d", config.tcp));
	VL_MISC_TRACE((" HCA                            : %s", config.hca_type));
	VL_MISC_TRACE((" Number of iterations           : %d", config.num_of_iter));
	VL_MISC_TRACE((" QP Type                        : %s", (VL_ibv_qp_type_str(config.qp_type))));
	VL_MISC_TRACE((" MAC                            : %s", config.mac));
	VL_MISC_TRACE((" Opcode                         : %s", (VL_ibv_wr_opcode_str(config.opcode))));
	VL_MISC_TRACE((" Ring-depth                     : %u", config.ring_depth));
	VL_MISC_TRACE((" msg size                       : %u", config.msg_sz));
	VL_MISC_TRACE((" Batch size                     : %u", config.batch_size));
	VL_MISC_TRACE((" Number of SGEs                 : %u", config.num_sge));
	VL_MISC_TRACE((" Use inline:                    : %s", bool_to_str(config.use_inl)));
	VL_MISC_TRACE((" Use post send method           : %d", config.send_method));
	VL_MISC_TRACE((" Wait before exit               : %s", bool_to_str(config.wait)));

	VL_MISC_TRACE((" --------------------------------------------------"));
}

/***********************************
* Function: process_arg.
************************************/
static int process_arg(
	IN		int opt_index,
	IN		char *equ_ptr,
	IN		int arr_size,
	IN		const struct VL_usage_descriptor_t *usage_desc_arr)
{
	/* process argument */

	switch (usage_descriptor[opt_index].case_code) {
	case HELP_CMD_CASE:
		VL_usage(1, arr_size, usage_desc_arr);
		exit(1);

	case INL_CMD_CASE:
		config.use_inl = 1;
		break;

	case RING_CMD_CASE:
		config.ring_depth = strtoul(equ_ptr, NULL, 0);
		if (!config.ring_depth) {
			VL_MISC_ERR(("Ring size cant be zero\n"));
			exit(1);
		}

		break;

	case SGE_CMD_CASE:
		config.num_sge = strtoul(equ_ptr, NULL, 0);
		if (!config.num_sge) {
			VL_MISC_ERR(("Number of SGEs cant be zero\n"));
			exit(1);
		}

		break;

	case NUM_OF_ITER_CMD_CASE:
		config.num_of_iter = strtoul(equ_ptr, NULL, 0);
		break;

	case HOST_CMD_CASE:
		config.hca_type = equ_ptr;
		break;

	case MAC_CMD_CASE:
		strcpy(config.mac, equ_ptr);
		break;

	case WAIT_CMD_CASE:
		config.wait = 1;
		break;

	case DAEMON_CMD_CASE:
		config.is_daemon = 1;
		break;

	case IP_CMD_CASE:
		strcpy(config.ip, equ_ptr);
		break;

	case TCP_CMD_CASE:
		config.tcp = strtoul(equ_ptr, NULL, 0);
		break;

	case BATCH_CMD_CASE:
		config.batch_size = strtoul(equ_ptr, NULL, 0);
		break;

	case SND_MTD_CMD_CASE:
		if (!strcmp("OLD",equ_ptr))
			config.send_method = METHOD_OLD;
		else if (!strcmp("NEW",equ_ptr))
			config.send_method = METHOD_NEW;
		else if (!strcmp("MIX",equ_ptr))
			config.send_method = METHOD_MIX;
		else {
			VL_MISC_ERR(("Unsupported post send method %s\n", equ_ptr));
			exit(1);
		}
		break;

	case QP_TYPE_CMD_CASE:
		if (!strcmp("RC",equ_ptr)) {
			config.qp_type = IBV_QPT_RC;
		} else if (!strcmp("DC",equ_ptr)) {
                        config.qp_type = IBV_QPT_DRIVER;
		} else if (!strcmp("UD",equ_ptr)) {
			config.qp_type = IBV_QPT_UD;
		} else if (!strcmp("RAW",equ_ptr)) {
			config.qp_type = IBV_QPT_RAW_PACKET;
		} else if (!strcmp("XRC",equ_ptr)) {
			if (config.is_daemon)
				config.qp_type = IBV_QPT_XRC_RECV;
			else
				config.qp_type = IBV_QPT_XRC_SEND;
		} else {
			VL_MISC_ERR(("Unsupported QP Transport Service Type %s\n", equ_ptr));
			exit(1);
		}
                break;

	case OP_CMD_CASE:
		if (!strcmp("SEND",equ_ptr)) {
			config.opcode = IBV_WR_SEND;
		} else if (!strcmp("SEND_IMM",equ_ptr)) {
			config.opcode = IBV_WR_SEND_WITH_IMM;
		} else if (!strcmp("WRITE",equ_ptr)) {
			config.opcode = IBV_WR_RDMA_WRITE;
		} else if (!strcmp("WRITE_IMM",equ_ptr)) {
			config.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
		} else if (!strcmp("READ",equ_ptr)) {
			config.opcode = IBV_WR_RDMA_READ;
		} else if (!strcmp("ATOMIC_FA",equ_ptr)) {
			config.opcode = IBV_WR_ATOMIC_FETCH_AND_ADD;
		} else if (!strcmp("ATOMIC_CS",equ_ptr)) {
			config.opcode = IBV_WR_ATOMIC_CMP_AND_SWP;
		} else if (!strcmp("EXT_ATOMIC_FA",equ_ptr)) {
			config.ext_atomic = 1;
			config.opcode = IBV_WR_ATOMIC_FETCH_AND_ADD;
		} else if (!strcmp("EXT_ATOMIC_CS",equ_ptr)) {
			config.ext_atomic = 1;
			config.opcode = IBV_WR_ATOMIC_CMP_AND_SWP;
		} else if (!strcmp("BIND",equ_ptr)) {
			config.opcode = IBV_WR_BIND_MW;
		} else if (!strcmp("LOCAL_INV",equ_ptr)) {
			config.opcode = IBV_WR_LOCAL_INV;
		} else if (!strcmp("SEND_INV",equ_ptr)) {
			config.opcode = IBV_WR_SEND_WITH_INV;
		} else {
			VL_MISC_ERR(("Unsupported opcode %s\n", equ_ptr));
			exit(1);
		}
                break;

	default:
		VL_MISC_ERR(("unknown parameter is the switch %s\n", equ_ptr));
		exit(4);
	}

	return 0;
}



/***********************************
* Function: parse_params.
************************************/
static int parse_params(
	IN		int argc,
	IN		char **argv)
{
	int rc;

	if (argc == 1) {
		VL_MISC_ERR((" Sorry , you must enter some data."
			     " type -h for help. "));
		exit(1);
	}

	rc = VL_parse_argv(argc, argv,
			   (sizeof(usage_descriptor)/sizeof(struct VL_usage_descriptor_t)),
			   (const struct VL_usage_descriptor_t *)(usage_descriptor),
			   (const VL_process_arg_func_t)process_arg);
	return rc;
}

/***********************************
* Function: main.
************************************/
int main(
	IN		int argc,
	IN		char *argv[])
{
	struct resources_t resource = {
		.sock = {
			.ip = "127.0.0.1",
			.port = 15000
		},
		.fd = -1
	};
	int rc = SUCCESS;

	rc = parse_params(argc, argv);
	CHECK_RC(rc, "parse_params");

	strcpy(resource.sock.ip, config.ip);

	rc = force_configurations_dependencies();
	CHECK_RC(rc, "force_configurations_dependencies");

	print_config();

	rc = resource_alloc(&resource);
	CHECK_RC(rc, "resource_alloc");

	rc = resource_init(&resource);
	CHECK_RC(rc, "resource_init");

	rc = sync_configurations(&resource);
	CHECK_RC(rc, "sync_configurations");

	rc = init_connection(&resource);
	CHECK_RC(rc, "init_connection");

	rc = sync_post_connection(&resource);
	CHECK_RC(rc, "sync_post_connection");

	rc = do_test(&resource);
	CHECK_RC(rc, "do_test");

	if (!config.is_daemon) {
		rc = print_results(&resource);
		CHECK_RC(rc, "print_results");
	}

cleanup:
	if (config.wait)
		VL_keypress_wait();

	if (resource_destroy(&resource) != SUCCESS)
		rc = FAIL;

	VL_print_test_status(rc);

	return rc;
}

