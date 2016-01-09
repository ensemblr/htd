/* 
 * File:   BucketEliminationGraphDecompositionAlgorithm.cpp
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

#ifndef HTD_HTD_BUCKETELIMINATIONGRAPHDECOMPOSITIONALGORITHM_CPP
#define	HTD_HTD_BUCKETELIMINATIONGRAPHDECOMPOSITIONALGORITHM_CPP

#include <htd/Globals.hpp>
#include <htd/Helpers.hpp>
#include <htd/BucketEliminationGraphDecompositionAlgorithm.hpp>
#include <htd/GraphLabeling.hpp>
#include <htd/ILabelingFunction.hpp>
#include <htd/OrderingAlgorithmFactory.hpp>

#include <algorithm>
#include <cstdarg>
#include <memory>
#include <stack>
#include <stdexcept>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

htd::BucketEliminationGraphDecompositionAlgorithm::BucketEliminationGraphDecompositionAlgorithm(void) : labelingFunctions_(), postProcessingOperations_()
{

}

htd::BucketEliminationGraphDecompositionAlgorithm::BucketEliminationGraphDecompositionAlgorithm(const std::vector<htd::IDecompositionManipulationOperation *> & manipulationOperations) : labelingFunctions_(), postProcessingOperations_()
{
    setManipulationOperations(manipulationOperations);
}

htd::BucketEliminationGraphDecompositionAlgorithm::~BucketEliminationGraphDecompositionAlgorithm()
{
    for (auto & labelingFunction : labelingFunctions_)
    {
        delete labelingFunction;
    }

    for (auto & postProcessingOperation : postProcessingOperations_)
    {
        delete postProcessingOperation;
    }
}

htd::IGraphDecomposition * htd::BucketEliminationGraphDecompositionAlgorithm::computeDecomposition(const htd::IHypergraph & graph) const
{
    return computeDecomposition(graph, std::vector<htd::IDecompositionManipulationOperation *>());
}

htd::IGraphDecomposition * htd::BucketEliminationGraphDecompositionAlgorithm::computeDecomposition(const htd::IHypergraph & graph, const std::vector<htd::IDecompositionManipulationOperation *> & manipulationOperations) const
{
    htd::IMutableGraphDecomposition * ret = computeMutableDecomposition(graph);

    std::vector<htd::ILabelingFunction *> labelingFunctions;

    std::vector<htd::IGraphDecompositionManipulationOperation *> postProcessingOperations;

    for (htd::IDecompositionManipulationOperation * operation : manipulationOperations)
    {
        htd::ILabelingFunction * labelingFunction = dynamic_cast<htd::ILabelingFunction *>(operation);

        if (labelingFunction != nullptr)
        {
            labelingFunctions.push_back(labelingFunction);
        }

        htd::IGraphDecompositionManipulationOperation * manipulationOperation = dynamic_cast<htd::IGraphDecompositionManipulationOperation *>(operation);

        if (manipulationOperation != nullptr)
        {
            postProcessingOperations.push_back(manipulationOperation);
        }
    }

    if (ret != nullptr)
    {
        for (const auto & operation : postProcessingOperations_)
        {
            operation->apply(*ret);
        }

        for (const auto & operation : postProcessingOperations)
        {
            operation->apply(*ret);
        }

        for (const auto & labelingFunction : labelingFunctions_)
        {
            for (htd::vertex_t vertex : ret->vertices())
            {
                htd::ILabelCollection * labelCollection = ret->labelings().exportVertexLabelCollection(vertex);

                htd::ILabel * newLabel = labelingFunction->computeLabel(ret->bagContent(vertex), *labelCollection);

                delete labelCollection;

                ret->setVertexLabel(labelingFunction->name(), vertex, newLabel);
            }
        }

        for (const auto & labelingFunction : labelingFunctions)
        {
            for (htd::vertex_t vertex : ret->vertices())
            {
                htd::ILabelCollection * labelCollection = ret->labelings().exportVertexLabelCollection(vertex);

                htd::ILabel * newLabel = labelingFunction->computeLabel(ret->bagContent(vertex), *labelCollection);

                delete labelCollection;

                ret->setVertexLabel(labelingFunction->name(), vertex, newLabel);
            }
        }
    }

    return ret;
}

htd::IGraphDecomposition * htd::BucketEliminationGraphDecompositionAlgorithm::computeDecomposition(const htd::IHypergraph & graph, int manipulationOperationCount, ...) const
{
    va_list arguments;

    va_start(arguments, manipulationOperationCount);

    std::vector<htd::IDecompositionManipulationOperation *> manipulationOperations;

    for (int manipulationOperationIndex = 0; manipulationOperationIndex < manipulationOperationCount; manipulationOperationIndex++)
    {
        manipulationOperations.push_back(va_arg(arguments, htd::IDecompositionManipulationOperation *));
    }

    return computeDecomposition(graph, manipulationOperations);
}

void htd::BucketEliminationGraphDecompositionAlgorithm::setManipulationOperations(const std::vector<htd::IDecompositionManipulationOperation *> & manipulationOperations)
{
    labelingFunctions_.clear();

    postProcessingOperations_.clear();

    for (htd::IDecompositionManipulationOperation * operation : manipulationOperations)
    {
        htd::ILabelingFunction * labelingFunction = dynamic_cast<htd::ILabelingFunction *>(operation);

        if (labelingFunction != nullptr)
        {
            labelingFunctions_.push_back(labelingFunction);
        }

        htd::IGraphDecompositionManipulationOperation * manipulationOperation = dynamic_cast<htd::IGraphDecompositionManipulationOperation *>(operation);

        if (manipulationOperation != nullptr)
        {
            postProcessingOperations_.push_back(manipulationOperation);
        }
    }
}

htd::BucketEliminationGraphDecompositionAlgorithm * htd::BucketEliminationGraphDecompositionAlgorithm::clone(void) const
{
    std::vector<htd::IDecompositionManipulationOperation *> manipulationOperations;

    for (const auto & labelingFunction : labelingFunctions_)
    {
        manipulationOperations.push_back(labelingFunction->clone());
    }

    for (const auto & postProcessingOperation : postProcessingOperations_)
    {
        manipulationOperations.push_back(postProcessingOperation->clone());
    }

    return new BucketEliminationGraphDecompositionAlgorithm(manipulationOperations);
}

htd::IMutableGraphDecomposition * htd::BucketEliminationGraphDecompositionAlgorithm::computeMutableDecomposition(const htd::IHypergraph & graph) const
{
    htd::IMutableGraphDecomposition * ret = nullptr;

    std::size_t size = graph.vertexCount();

    if (size > 0)
    {
        htd::IOrderingAlgorithm * algorithm = htd::OrderingAlgorithmFactory::instance().getOrderingAlgorithm();

        if (algorithm == nullptr)
        {
            throw std::logic_error("htd::IMutableGraphDecomposition * htd::BucketEliminationGraphDecompositionAlgorithm::computeMutableDecomposition(const htd::IHypergraph &) const");
        }

        const htd::ConstCollection<htd::vertex_t> & ordering = algorithm->computeOrdering(graph);

        delete algorithm;

        if (ordering.size() == size)
        {
            std::vector<htd::index_t> indices(size);

            //std::vector<htd::vertex_t> vertexLabels(size, htd::Vertex::UNKNOWN);

            std::vector<htd::vertex_container> buckets(size);

            std::vector<htd::id_t> relevantBuckets;

            std::unordered_set<htd::vertex_t> isolatedVertices(ordering.begin(), ordering.end());

            DEBUGGING_CODE(std::cout << "Ordering:" << std::endl;

            for (htd::vertex_t vertex : ordering)
            {
                std::cout << vertex << std::endl;
            })

            std::size_t index = 0;

            for (htd::vertex_t vertex : ordering)
            {
                indices[vertex - htd::Vertex::FIRST] = index++;
            }

            for (std::size_t index = 0; index < size; index++)
            {
                buckets[index].push_back(index + htd::Vertex::FIRST);
            }

            for (const htd::Hyperedge & edge : graph.hyperedges())
            {
                htd::vertex_container elements = htd::vertex_container(edge.begin(), edge.end());

                std::sort(elements.begin(), elements.end());

                elements.erase(std::unique(elements.begin(), elements.end()), elements.end());

                htd::vertex_t minimumVertex = getMinimumVertex(elements, indices);

                auto & selectedBucket = buckets[minimumVertex - htd::Vertex::FIRST];

                std::vector<htd::vertex_t> newBucketContent;
                newBucketContent.reserve(selectedBucket.size());

                /*
                if (vertexLabels[minimumVertex - htd::Vertex::FIRST] == htd::unknown_id)
                {
                    relevantBuckets.push_back(minimumVertex);
                }
                */

                if (elements.size() > 1)
                {
                    for (htd::vertex_t vertex : elements)
                    {
                        isolatedVertices.erase(vertex);
                    }
                }

                //vertexLabels[minimumVertex - htd::Vertex::FIRST] = minimumVertex;

                std::set_union(selectedBucket.begin(), selectedBucket.end(), elements.begin(), elements.end(), std::back_inserter(newBucketContent));

                std::swap(selectedBucket, newBucketContent);
            }

            if (isolatedVertices.size() > 0)
            {
                for (htd::vertex_t vertex : isolatedVertices)
                {
                    relevantBuckets.push_back(vertex);
                }
            }

            //TODO
            relevantBuckets.clear();
            std::copy(ordering.begin(), ordering.end(), std::back_inserter(relevantBuckets));

            std::sort(relevantBuckets.begin(), relevantBuckets.end());

            DEBUGGING_CODE(std::cout << std::endl << "Buckets:" << std::endl;
            for (std::size_t index = 0; index < size; index++)
            {
                std::cout << "   Bucket " << index + htd::Vertex::FIRST << ": ";
                htd::print(buckets[index], false);
                std::cout << std::endl;
            }

            std::cout << std::endl << "Relevant Buckets:" << std::endl;
            for (htd::id_t bucket : relevantBuckets)
            {
                std::cout << "   Bucket " << bucket << ": ";
                htd::print(buckets[bucket - htd::Vertex::FIRST], false);
                std::cout << std::endl;
            }

            std::cout << std::endl << "Connections:" << std::endl;
            )

            std::size_t edgeCount = 0;

            //TODO Implement and use htd::NamedGraph<int,int>!
            //htd::NamedGraph<int, int> result = htd::GraphFactory::instance().getGraph();

            std::vector<htd::vertex_t> tmp;
            std::vector<std::vector<htd::vertex_t>> neighbors(size);

            for (htd::index_t index = 0; index < size; index++)
            {
                tmp.clear();

                htd::vertex_t selection = ordering[index];

                DEBUGGING_CODE(std::cout << std::endl << "   Processing bucket " << selection << " ..." << std::endl;)

                for (htd::vertex_t vertex : buckets[selection - htd::Vertex::FIRST])
                {
                    if (vertex != selection)
                    {
                        tmp.push_back(vertex);
                    }
                }

                if (tmp.size() > 0)
                {
                    DEBUGGING_CODE(
                        std::cout << "      Bucket " << selection << ": ";
                        htd::print(tmp, false);
                        std::cout << std::endl;
                    )

                    htd::vertex_t minimumVertex = getMinimumVertex(tmp, indices);

                    DEBUGGING_CODE(
                        std::cout << "      Minimum Vertex: " << minimumVertex << std::endl;

                        if (minimumVertex < selection)
                        {
                            std::cout << "      Connection: " << minimumVertex << " - " << selection << std::endl;
                        }
                        else
                        {
                            std::cout << "      Connection: " << selection << " - " << minimumVertex << std::endl;
                        }
                    )

                    auto & selectedBucket = buckets[minimumVertex - htd::Vertex::FIRST];

                    std::vector<htd::vertex_t> newBucketContent;
                    newBucketContent.reserve(selectedBucket.size());

                    std::set_union(selectedBucket.begin(), selectedBucket.end(), tmp.begin(), tmp.end(), std::back_inserter(newBucketContent));

                    std::swap(selectedBucket, newBucketContent);

                    //TODO
                    /*
                    htd::id_t selectionLabel = labels[selection];
                    htd::id_t minimumVertexLabel = labels[minimumVertex];

                    if (minimumVertexLabel != htd::unknown_id && selectionLabel != htd::unknown_id)
                    {
                        ++edgeCount;

                        neighbors[selectionLabel].push_back(minimumVertexLabel);
                        neighbors[minimumVertexLabel].push_back(selectionLabel);
                    }
                    */

                    neighbors[selection - htd::Vertex::FIRST].push_back(minimumVertex);
                    neighbors[minimumVertex - htd::Vertex::FIRST].push_back(selection);
                }
            }

            DEBUGGING_CODE(std::cout << std::endl << "Buckets:" << std::endl;
            for (std::size_t index = 0; index < size; index++)
            {
                std::cout << "   Bucket " << index + htd::Vertex::FIRST << ": ";
                htd::print(buckets[index], false);
                std::cout << std::endl;
            })

            DEBUGGING_CODE(std::cout << std::endl << "Relevant Buckets:" << std::endl;
            for (htd::id_t bucket : relevantBuckets)
            {
                std::cout << "   Bucket " << bucket << ": ";
                htd::print(buckets[bucket - htd::Vertex::FIRST], false);
                std::cout << std::endl;
            })

            //TODO Selection of root
            htd::vertex_t root = relevantBuckets[0];

            if (edgeCount < relevantBuckets.size() - 1)
            {
                std::vector<htd::index_t> bucketIndices(size);

                std::vector<htd::vertex_t> unreachableVertices;

                htd::index_t currentIndex = 0;

                for (htd::vertex_t bucket : relevantBuckets)
                {
                    bucketIndices[bucket - htd::Vertex::FIRST] = currentIndex;

                    ++currentIndex;
                }

                getUnreachableVertices(root, relevantBuckets, bucketIndices, neighbors, unreachableVertices);

                while (unreachableVertices.size() > 0)
                {
                    std::size_t bestOverlap = 0;
                    htd::vertex_t bestBucket = htd::Vertex::UNKNOWN;
                    htd::vertex_t currentBucket = unreachableVertices[0];

                    auto & currentBucketContent = buckets[currentBucket - htd::Vertex::FIRST];

                    std::vector<htd::vertex_t> reachableVertices;

                    getReachableVertices(currentBucket, relevantBuckets, bucketIndices, neighbors, reachableVertices);

                    auto it = reachableVertices.begin();
                    auto last = reachableVertices.end();

                    for (htd::vertex_t bucket : relevantBuckets)
                    {
                        while (it != last && bucket > *it)
                        {
                            it++;
                        }

                        if (it == last || bucket < *it)
                        {
                            auto & bucketContent = buckets[bucket - htd::Vertex::FIRST];

                            std::size_t currentOverlap = htd::compute_set_intersection_size(currentBucketContent.begin(), currentBucketContent.end(), bucketContent.begin(), bucketContent.end());

                            //TODO Keep options of same quality and select (randomly) from this pool?
                            if (currentOverlap > bestOverlap)
                            {
                                bestBucket = bucket;
                                bestOverlap = currentOverlap;
                            }
                        }
                    }

                    if (bestBucket == htd::Vertex::UNKNOWN)
                    {
                        bestBucket = root;
                    }

                    DEBUGGING_CODE(std::cout << std::endl << "Unreachable Vertices: " << unreachableVertices.size() << std::endl;)

                    if (unreachableVertices.size() > 1)
                    {
                        std::vector<htd::vertex_t> newUnreachableVertices;

                        std::set_difference(unreachableVertices.begin(), unreachableVertices.end(), reachableVertices.begin(), reachableVertices.end(), std::back_inserter(newUnreachableVertices));

                        unreachableVertices.clear();

                        std::swap(unreachableVertices, newUnreachableVertices);
                    }
                    else
                    {
                        unreachableVertices.clear();
                    }

                    neighbors[bestBucket - htd::Vertex::FIRST].push_back(currentBucket);
                    neighbors[currentBucket - htd::Vertex::FIRST].push_back(bestBucket);
                }
            }

            //ret = createRootedTreeDecomposition(relevantBuckets[root - htd::Vertex::FIRST], neighbors, buckets);
        }
    }
    else
    {
        //ret = htd::TreeDecompositionFactory::instance().getTreeDecomposition();
    }

    return ret;
}

htd::vertex_t htd::BucketEliminationGraphDecompositionAlgorithm::getMinimumVertex(const std::vector<htd::vertex_t> & vertices, const std::vector<htd::index_t> & vertexIndices) const
{
    htd::vertex_t ret = htd::Vertex::UNKNOWN;

    if (vertices.size() > 0)
    {
        std::size_t minimum = (std::size_t)-1;

        std::size_t currentIndex = (std::size_t)-1;

        for (htd::vertex_t vertex : vertices)
        {
            currentIndex = vertexIndices[vertex - htd::Vertex::FIRST];

            if (currentIndex < minimum)
            {
                ret = vertex;

                minimum = currentIndex;
            }
        }
    }
    else
    {
        throw std::out_of_range("htd::BucketEliminationGraphDecompositionAlgorithm::getMinimumVertex(const htd::Collection<htd::vertex_t> &, const std::vector<htd::index_t> &) const");
    }

    return ret;
}

void htd::BucketEliminationGraphDecompositionAlgorithm::getReachableVertices(htd::vertex_t start, const htd::vertex_container & vertices, const std::vector<htd::index_t> & vertexIndices, const std::vector<htd::vertex_container> & neighbors, htd::vertex_container & output) const
{
    std::size_t size = vertices.size();

    if (size > 0)
    {
        htd::vertex_t vertex = htd::Vertex::UNKNOWN;

        htd::vertex_container newVertices;
        htd::vertex_container tmpVertices;

        std::vector<bool> reachableVertices(size, false);

        reachableVertices[vertexIndices[start - htd::Vertex::FIRST]] = true;

        newVertices.push_back(start);

        output.push_back(start);

        while (newVertices.size() > 0)
        {
            std::swap(tmpVertices, newVertices);

            newVertices.clear();

            for (auto index : tmpVertices)
            {
                vertex = vertices[vertexIndices[index - htd::Vertex::FIRST]];

                for (htd::vertex_t neighbor : neighbors[vertex - htd::Vertex::FIRST])
                {
                    htd::index_t vertexIndex = vertexIndices[neighbor - htd::Vertex::FIRST];

                    if (!reachableVertices[vertexIndex])
                    {
                        reachableVertices[vertexIndex] = true;

                        output.push_back(neighbor);

                        newVertices.push_back(neighbor);
                    }
                }
            }
        }

        std::sort(output.begin(), output.end());
    }
}

void htd::BucketEliminationGraphDecompositionAlgorithm::getUnreachableVertices(htd::vertex_t start, const htd::vertex_container & vertices, const std::vector<htd::index_t> & vertexIndices, const std::vector<htd::vertex_container> & neighbors, htd::vertex_container & output) const
{
    std::size_t size = vertices.size();

    if (size > 0)
    {
        htd::vertex_container reachableVertices;

        getReachableVertices(start, vertices, vertexIndices, neighbors, reachableVertices);

        std::set_difference(vertices.begin(), vertices.end(), reachableVertices.begin(), reachableVertices.end(), std::back_inserter(output));
    }
}

#endif /* HTD_HTD_BUCKETELIMINATIONGRAPHDECOMPOSITIONALGORITHM_CPP */
