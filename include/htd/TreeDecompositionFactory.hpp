/*
 * File:   TreeDecompositionFactory.hpp
 *
 * Author: ABSEHER Michael (abseher@dbai.tuwien.ac.at)
 *
 * Copyright 2015-2016, Michael Abseher
 *    E-Mail: <abseher@dbai.tuwien.ac.at>
 *
 * This file is part of htd.
 *
 * htd is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * htd is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.

 * You should have received a copy of the GNU General Public License
 * along with htd.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HTD_HTD_TREEDECOMPOSITIONFACTORY_HPP
#define HTD_HTD_TREEDECOMPOSITIONFACTORY_HPP

#include <htd/Globals.hpp>
#include <htd/IMutableTreeDecomposition.hpp>
#include <htd/ITreeDecomposition.hpp>

namespace htd
{
    class TreeDecompositionFactory
    {
        public:
            ~TreeDecompositionFactory();

            static TreeDecompositionFactory & instance(void);

            htd::IMutableTreeDecomposition * getTreeDecomposition(void);

            htd::IMutableTreeDecomposition * getTreeDecomposition(const htd::ITreeDecomposition & original);

            void setConstructionTemplate(htd::IMutableTreeDecomposition * original);

            htd::IMutableTreeDecomposition & accessMutableTreeDecomposition(htd::ITreeDecomposition & original);

            const htd::IMutableTreeDecomposition & accessMutableTreeDecomposition(const htd::ITreeDecomposition & original);

        private:
            htd::IMutableTreeDecomposition * constructionTemplate_;

            TreeDecompositionFactory(void);

            TreeDecompositionFactory(const TreeDecompositionFactory & original);

            TreeDecompositionFactory & operator=(const TreeDecompositionFactory & original);
    };
}

#endif /* HTD_HTD_TREEDECOMPOSITIONFACTORY_HPP */
