# Copyright (c) 2021 Laird Connectivity
#
# Makelists file for the BL5340 Application Core Common firmware.
#
# SPDX-License-Identifier: Apache-2.0

target_sources_ifdef(CONFIG_BL5340_RPC_CLIENT app PRIVATE client/bl5340_rpc_client_handlers.c)
target_sources_ifdef(CONFIG_BL5340_RPC_CLIENT app PRIVATE client/bl5340_rpc_client_interface.c)
target_sources_ifdef(CONFIG_BL5340_RPC_SERVER app PRIVATE server/bl5340_rpc_server_interface.c)