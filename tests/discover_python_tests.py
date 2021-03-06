########################################################################################################################
#     mogli - molecular graph library                                                                                  #
#                                                                                                                      #
#     Copyright (C) 2016-2019  Martin S. Engler                                                                        #
#                                                                                                                      #
#     This program is free software: you can redistribute it and/or modify                                             #
#     it under the terms of the GNU Lesser General Public License as published                                         #
#     by the Free Software Foundation, either version 3 of the License, or                                             #
#     (at your option) any later version.                                                                              #
#                                                                                                                      #
#     This program is distributed in the hope that it will be useful,                                                  #
#     but WITHOUT ANY WARRANTY; without even the implied warranty of                                                   #
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                                                     #
#     GNU General Public License for more details.                                                                     #
#                                                                                                                      #
#     You should have received a copy of the GNU Lesser General Public License                                         #
#     along with this program.  If not, see <https://www.gnu.org/licenses/>.                                           #
########################################################################################################################

import unittest


def print_suite(suite):
    if hasattr(suite, '__iter__'):
        for x in suite:
            print_suite(x)
    else:
        print(suite.id())


if __name__ == '__main__':
    print_suite(unittest.defaultTestLoader.discover('.'))
