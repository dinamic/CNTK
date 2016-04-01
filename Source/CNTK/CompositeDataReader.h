//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.md file in the project root for full license information.
//

#pragma once

#include <map>
#include <string>
#include <future>
#include "DataReader.h"
#include "DataDeserializer.h"
#include "Transformer.h"
#include "Reader.h"
#include "Packer.h"

namespace Microsoft { namespace MSR { namespace CNTK {

// TODO: Temporary shim for the new readers, will be removed and responsibilities will be moved to different parts of CNTK.
// TODO: Currently binds together several deserializers, packer and randomizer. So that the actual reader developer has to provide deserializer only.
// TODO: Same code as in ReaderLib shim, the one in the ReaderLib will be deleted as the next step.
template <class ElemType>
class CompositeDataReader : public IDataReader, protected Plugin, public ScriptableObjects::Object
{
public:
    CompositeDataReader();

    // Currently we do not support BS configuration.
    virtual void Init(const ScriptableObjects::IConfigRecord& /*config*/) override
    {
        assert(false);
    }

    virtual void Init(const ConfigParameters& config) override;

    virtual void Destroy() override
    {
        delete this;
    }

    virtual void StartMinibatchLoop(size_t mbSize, size_t epoch, size_t requestedEpochSamples = requestDataSize) override;
    virtual void StartDistributedMinibatchLoop(size_t requestedMBSize, size_t epoch, size_t subsetNum, size_t numSubsets, size_t requestedEpochSamples) override;

    virtual bool SupportsDistributedMBRead() const override
    {
        return true;
    }

    virtual bool GetMinibatch(StreamMinibatchInputs& matrices) override;
    virtual bool DataEnd() override;
    void CopyMBLayoutTo(MBLayoutPtr) override;
    virtual size_t GetNumParallelSequences() override;

private:
    void CreateDeserializers(const ConfigParameters& readerConfig);
    IDataDeserializerPtr CreateDeserializer(const ConfigParameters& readerConfig);
    void StartEpoch(const EpochConfiguration& config);

    std::future<Minibatch> m_prefetchTask;
    bool m_endOfEpoch;

    MBLayoutPtr m_layout;

    std::map<std::wstring, size_t> m_nameToStreamId;
    std::vector<StreamDescriptionPtr> m_streams;
    launch m_launchType;

    std::vector<IDataDeserializerPtr> m_deserializers;

    // TODO: remove Transformer interface from randomizer.
    TransformerPtr m_randomizer;

    // TODO: Should be removed. We already have matrices on this level.
    // Should just get the corresponding pinned memory.
    MemoryProviderPtr m_provider;

    PackerPtr m_packer;

    bool m_frameMode;
    bool m_truncated;
};

}}}
