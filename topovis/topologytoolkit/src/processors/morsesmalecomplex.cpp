/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/topologytoolkit/processors/morsesmalecomplex.h>
#include <inviwo/topologytoolkit/utils/ttkutils.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/datastructures/geometry/meshram.h>

#include <warn/push>
#include <warn/ignore/all>
#include <ttk/core/base/morseSmaleComplex/MorseSmaleComplex.h>
#include <ttk/core/base/topologicalSimplification/TopologicalSimplification.h>
#include <ttk/core/base/ftmTree/FTMTree_MT.h>
#include <warn/pop>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo MorseSmaleComplex::processorInfo_{
    "org.inviwo.MorseSmaleComplex",      // Class identifier
    "Morse-Smale Complex",               // Display name
    "Topology",                          // Category
    CodeState::Experimental,             // Code state
    "CPU, Topology, TTK, Contour Tree",  // Tags
};
const ProcessorInfo MorseSmaleComplex::getProcessorInfo() const { return processorInfo_; }

MorseSmaleComplex::MorseSmaleComplex()
    : Processor() {

    addPort(inport_);
    addPort(outport_);
}

void MorseSmaleComplex::process() {
    // Save input and properties needed to calculate ttk contour tree to local variables
    const auto inportData = inport_.getData();

    // construction of ttk contour tree
    auto computeMsc = [this, inportData](const auto buffer) {
        using ValueType = util::PrecisionValueType<decltype(buffer)>;
        using PrimitiveType = typename DataFormat<ValueType>::primitive;

        auto mscData = std::make_shared<topology::MorseSmaleComplexData>();
        mscData->triangulation = inportData;

        std::vector<int> offsets(inportData->getOffsets());

        ttk::MorseSmaleComplex morseSmaleComplex;
        morseSmaleComplex.setupTriangulation(
            const_cast<ttk::Triangulation *>(&inportData->getTriangulation()));
        // FIXME: ttk::MorseSmaleComplex has some issues with correct constness
        morseSmaleComplex.setInputScalarField(const_cast<PrimitiveType *>(buffer->getDataContainer().data()));
        morseSmaleComplex.setInputOffsets(offsets.data());

        mscData->setMSCOutput(morseSmaleComplex,
                              inportData->getTriangulation().getNumberOfVertices());

        morseSmaleComplex.execute<PrimitiveType, int>();

        return mscData;
    };

    dispatchPool([this, inportData, computeMsc]() {
        auto mscData = inportData->getScalarValues()
                           ->getRepresentation<BufferRAM>()
                           ->dispatch<std::shared_ptr<topology::MorseSmaleComplexData>,
                                      dispatching::filter::Scalars>(computeMsc);

        dispatchFront([this, mscData]() { outport_.setData(mscData); });
    });
}

}  // namespace inviwo
