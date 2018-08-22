<!---
 Copyright (C) 2018 Jiajie Chen
 
 This file is part of l2tpv3udptap.
 
 l2tpv3udptap is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 l2tpv3udptap is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with l2tpv3udptap.  If not, see <http://www.gnu.org/licenses/>.
 
-->

l2tpv3udptap
================================

A L2TPv3 over UDP implementation for macOS. Requires [tuntaposx](http://tuntaposx.sourceforge.net/). Tunnelblick also provides a signed version of it.


Usage
================================

```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./l2tpv3udptap # Usage help
Usage: l2tpv3udptap [tap_if] [local_ip] [remote_ip] [port] [peer_port] [session_id] [peer_session_id] [cookie] [peer_cookie]
	equivalent to: ip l2tp add tunnel remote [remote_ip] local [local_ip] tunnel_id 0 peer_tunnel_id 0 encap udp
	             : ip l2tp add session name [tap_if] tunnel_id 0 session_id [session_id] peer_session_id [peer_session_id] cookie [cookie] peer_cookie [peer_cookie]
```