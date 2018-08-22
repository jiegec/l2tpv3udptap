# Copyright (C) 2018 Jiajie Chen
# 
# This file is part of l2tpv3udpmac.
# 
# l2tpv3udpmac is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# l2tpv3udpmac is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with l2tpv3udpmac.  If not, see <http:#www.gnu.org/licenses/>.
# 

cmake_minimum_required(VERSION 3.8)
project(l2tpv3udpmac)

add_executable(l2tpv3udpmac main.cpp)
target_compile_features(l2tpv3udpmac PRIVATE cxx_std_11)
