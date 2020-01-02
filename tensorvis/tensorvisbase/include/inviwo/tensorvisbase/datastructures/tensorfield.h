#pragma once

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/datastructures/datamapper.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/util/indexmapper.h>
#include <inviwo/tensorvisbase/tensorvisbasemoduledefine.h>
#include <inviwo/tensorvisbase/util/tensorutil.h>
#include <inviwo/tensorvisbase/util/misc.h>
#include <Eigen/Dense>
#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/tensorvisbase/datastructures/attributes.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/datastructures/column.h>
#include <type_traits>
#include <optional>
#include <memory>

namespace inviwo {
/**
 * \class TensorField
 * \brief Base data structure for tensorfields.
 */
template <unsigned int N, typename precision>
class TensorField : public StructuredGridEntity<N> {
public:
    using sizeN_t = glm::vec<N, std::size_t, glm::defaultp>;
    using matN = glm::mat<N, N, precision, glm::defaultp>;
    using matNb = glm::mat<N + 1, N + 1, precision, glm::defaultp>;
    using vecN = glm::vec<N, precision, glm::defaultp>;

    TensorField() = delete;

    TensorField(const sizeN_t& dimensions, const std::vector<matN>& tensors);
    TensorField(const sizeN_t& dimensions, std::shared_ptr<const std::vector<matN>> tensors);

    TensorField(const sizeN_t& dimensions, const std::vector<matN>& tensors,
                const DataFrame& metaData);
    TensorField(const sizeN_t& dimensions, std::shared_ptr<const std::vector<matN>> tensors,
                std::shared_ptr<const DataFrame> metaData);

    /**
     * NOTE: This method creates a shallow copy, i.e. the tensors and the meta data are
     * not copied. Rather, the copy points towards the same data as the input field. If you need a
     * deep copy, use the deepCopy method.
     */
    TensorField(const TensorField<N, precision>& tf);

    TensorField& operator=(const TensorField&) = delete;

    // Destructors
    virtual ~TensorField() = default;

    /**
     * NOTE: This method creates a shallow copy, i.e. the tensors and the meta data are
     * not copied. Rather, the copy points towards the same data as the input field. If you need a
     * deep copy, use the deepCopy method.
     */
    virtual TensorField<N, precision>* clone() const override;

    /**
     * NOTE: This method constructor creates a deep copy, i.e. the tensors and the meta data are
     * copied. If you need a shallow copy, use the copy constructor or clone method.
     */
    virtual std::shared_ptr<TensorField<N, precision>> deepCopy() const;

    std::string getDataInfo() const;

    template <bool useMask = false>
    const auto at(sizeN_t position) const;

    template <bool useMask = false>
    const auto at(size_t index) const;

    sizeN_t getDimensions() const final { return dimensions_; }

    template <typename T = float>
    glm::vec<N, T> getExtents() const;

    void setExtents(const glm::vec<N, float>& extents);

    template <typename T = size_t>
    glm::vec<N, T> getBounds() const;

    template <typename T = float>
    glm::vec<N, T> getSpacing() const;

    size_t getSize() const { return size_; }

    matNb getBasisAndOffset() const;

    std::shared_ptr<const std::vector<matN>> tensors() const;

    void setMask(const std::vector<glm::uint8>& mask) { binaryMask_ = mask; }
    const std::vector<glm::uint8>& getMask() const { return binaryMask_; }

    void setTensors(std::shared_ptr<const std::vector<matN>> tensors);
    void setMetaData(std::shared_ptr<const DataFrame> metaData);

    /*
    If the tensor field has a mask, this method return the number of 1s in it -
    telling you how many of the positions in the tensor field are defined.
    */
    int getNumDefinedEntries() const;

    /**
     * Data map for the eigen values of the tensor field.
     * 0 := major, 1 := middle, 2 := minor
     */
    std::array<DataMapper, N> dataMapEigenValues_;
    /**
     * Data map for the eigen vectors of the tensor field (global min/max of all vector components).
     * 0 := major, 1 := middle, 2 := minor
     */
    std::array<DataMapper, N> dataMapEigenVectors_;

    bool hasMask() const { return binaryMask_.size() == size_; }

    const util::IndexMapper<N>& indexMapper() const { return indexMapper_; }

    /**
     * Perform lookup as to whether the specified meta data is available for the tensor field.
     * HINT: If it is not, you might want to add a meta data processor to your network to calculate
     * the desired meta data.
     */
    template <typename T>
    bool hasMetaData() const;

    /**
     * Tensor field meta data is stored in a DataFrame. If avaliable, this method returns the column
     * for the meta data specified by T (see attributes.h). Nullopt otherwise.
     */
    template <typename T>
    std::optional<std::shared_ptr<const Column>> getMetaData() const;

    /**
     * This method returns the underlying container for the meta data specified by T (see
     * attributes.h). NOTE: This method does not check whether or not the specified meta data is
     * available.
     */
    template <typename T>
    const std::vector<typename T::value_type>& getMetaDataContainer() const;

    std::shared_ptr<const DataFrame> metaData() const { return metaData_; }

protected:
    sizeN_t dimensions_;
    util::IndexMapper<N> indexMapper_;
    std::shared_ptr<const std::vector<matN>> tensors_;
    size_t size_;
    std::shared_ptr<const DataFrame> metaData_;

    std::vector<glm::uint8> binaryMask_;
};

template <unsigned int N, typename precision>
inline TensorField<N, precision>::TensorField(const sizeN_t& dimensions,
                                              const std::vector<matN>& tensors)
    : dimensions_(dimensions)
    , indexMapper_(util::IndexMapper<N>(dimensions))
    , tensors_(std::make_shared<std::vector<matN>>(tensors))
    , size_(glm::compMul(dimensions))
    , metaData_(std::make_shared<DataFrame>()) {}

template <unsigned int N, typename precision>
inline TensorField<N, precision>::TensorField(const sizeN_t& dimensions,
                                              std::shared_ptr<const std::vector<matN>> tensors)
    : dimensions_(dimensions)
    , indexMapper_(util::IndexMapper<N>(dimensions))
    , tensors_(std::make_shared<std::vector<matN>>(*tensors))
    , size_(glm::compMul(dimensions))
    , metaData_(std::make_shared<DataFrame>()) {}

template <unsigned int N, typename precision>
inline TensorField<N, precision>::TensorField(const sizeN_t& dimensions,
                                              const std::vector<matN>& tensors,
                                              const DataFrame& metaData)
    : dimensions_(dimensions)
    , indexMapper_(util::IndexMapper<N>(dimensions))
    , tensors_(std::make_shared<std::vector<matN>>(tensors))
    , size_(glm::compMul(dimensions))
    , metaData_(std::make_shared<DataFrame>(metaData)) {}

template <unsigned int N, typename precision>
inline TensorField<N, precision>::TensorField(const sizeN_t& dimensions,
                                              std::shared_ptr<const std::vector<matN>> tensors,
                                              std::shared_ptr<const DataFrame> metaData)
    : dimensions_(dimensions)
    , indexMapper_(util::IndexMapper<N>(dimensions))
    , tensors_(std::make_shared<std::vector<matN>>(*tensors))
    , size_(glm::compMul(dimensions))
    , metaData_(metaData) {}

template <unsigned int N, typename precision>
inline TensorField<N, precision>::TensorField(const TensorField<N, precision>& tf)
    : StructuredGridEntity<3>()
    , dataMapEigenValues_(tf.dataMapEigenValues_)
    , dataMapEigenVectors_(tf.dataMapEigenVectors_)
    , dimensions_(tf.dimensions_)
    , indexMapper_(util::IndexMapper<N>(dimensions_))
    , tensors_(tf.tensors_)
    , size_(tf.size_)
    , metaData_(tf.metaData_)
    , binaryMask_(tf.binaryMask_) {
    this->setOffset(tf.getOffset());
    this->setBasis(tf.getBasis());
}

template <unsigned int N, typename precision>
inline TensorField<N, precision>* TensorField<N, precision>::clone() const {
    return new TensorField<N, precision>(*this);
}

template <unsigned int N, typename precision>
inline std::shared_ptr<TensorField<N, precision>> TensorField<N, precision>::deepCopy() const {
    auto tf = std::make_shared<TensorField<N, precision>>(*this);

    tf->setTensors(std::make_shared<std::vector<matN>>(*tensors_));
    tf->setMetaData(std::make_shared<DataFrame>(*metaData_));

    return tf;
}

template <unsigned int N, typename precision>
inline std::string TensorField<N, precision>::getDataInfo() const {
    std::stringstream ss;
    ss << "<table border='0' cellspacing='0' cellpadding='0' "
          "style='border-color:white;white-space:pre;'>/n"
       << tensorutil::getHTMLTableRowString("Type", "3D tensor field")
       << tensorutil::getHTMLTableRowString("Number of tensors", tensors_->size())
       << tensorutil::getHTMLTableRowString("Dimensions", dimensions_)
       << tensorutil::getHTMLTableRowString("Max major field eigenvalue",
                                            dataMapEigenValues_[0].valueRange.y)
       << tensorutil::getHTMLTableRowString("Min major field eigenvalue",
                                            dataMapEigenValues_[0].valueRange.x)
       << tensorutil::getHTMLTableRowString("Max intermediate field eigenvalue",
                                            dataMapEigenValues_[1].valueRange.y)
       << tensorutil::getHTMLTableRowString("Min intermediate field eigenvalue",
                                            dataMapEigenValues_[1].valueRange.x)
       << tensorutil::getHTMLTableRowString("Max minor field eigenvalue",
                                            dataMapEigenValues_[2].valueRange.y)
       << tensorutil::getHTMLTableRowString("Min minor field eigenvalue",
                                            dataMapEigenValues_[2].valueRange.x)
       << tensorutil::getHTMLTableRowString("Extends", getExtents())

       << "</table>";
    return ss.str();
}

template <unsigned int N, typename precision>
template <bool useMask>
inline const auto TensorField<N, precision>::at(sizeN_t position) const {
    return this->at<useMask>(indexMapper_(position));
}

template <unsigned int N, typename precision>
template <bool useMask>
inline const auto TensorField<N, precision>::at(size_t index) const {
    if constexpr (useMask) {
        return std::make_pair<const bool, const matN&>(binaryMask_[index],
                                                       tensors_->operator[](index));
    } else {
        return tensors_->operator[](index);
    }
}

template <unsigned int N, typename precision>
template <typename T>
inline glm::vec<N, T> TensorField<N, precision>::getExtents() const {
    const auto basis = this->getBasis();

    glm::vec<N, T> extents{};

    for (unsigned int i{0}; i < N; ++i) {
        extents[i] = glm::length(basis[i]);
    }

    return extents;
}

template <unsigned int N, typename precision>
inline void TensorField<N, precision>::setExtents(const glm::vec<N, float>& extents) {
    auto basis = this->getBasis();

    for (unsigned int i{0}; i < N; ++i) {
        basis[i] = glm::normalize(basis[i]) * extents[i];
    }

    this->setBasis(basis);
}

template <unsigned int N, typename precision>
template <typename T>
inline glm::vec<N, T> TensorField<N, precision>::getBounds() const {
    const auto b = this->getDimensions() - sizeN_t(1);
    return glm::vec<N, T>(glm::max(b, sizeN_t(1)));
}

template <unsigned int N, typename precision>
template <typename T>
inline glm::vec<N, T> TensorField<N, precision>::getSpacing() const {
    return getExtents<T>() / getBounds<T>();
}

template <unsigned int N, typename precision>
inline typename TensorField<N, precision>::matNb TensorField<N, precision>::getBasisAndOffset()
    const {
    auto basis = this->getBasis();
    auto offset = this->getOffset();

    matNb modelMatrix;

    for (unsigned int i{0}; i < N - 1; ++i) {
        modelMatrix[i] = glm::vec<N + 1, precision>(basis[i], precision(0));
    }

    modelMatrix[N - 1] = glm::vec<N + 1, precision>(offset, precision(1));

    return modelMatrix;
}

template <unsigned int N, typename precision>
inline std::shared_ptr<const std::vector<typename TensorField<N, precision>::matN>>
TensorField<N, precision>::tensors() const {
    return tensors_;
}

template <unsigned int N, typename precision>
inline void TensorField<N, precision>::setTensors(
    std::shared_ptr<const std::vector<typename TensorField<N, precision>::matN>> tensors) {
    tensors_ = tensors;
}

template <unsigned int N, typename precision>
inline void TensorField<N, precision>::setMetaData(std::shared_ptr<const DataFrame> metaData) {
    metaData_ = metaData;
}

template <unsigned int N, typename precision>
inline int TensorField<N, precision>::getNumDefinedEntries() const {
    return static_cast<int>(std::count(std::begin(binaryMask_), std::end(binaryMask_), 1));
}

template <unsigned int N, typename precision>
template <typename T>
inline bool TensorField<N, precision>::hasMetaData() const {
    if constexpr (std::is_base_of_v<attributes::AttributeBase, T>) {
        const auto& headers = metaData_->getHeaders();
        const auto name = std::string(T::identifier);
        auto pred = [name](const std::pair<std::string, const DataFormatBase*>& pair) -> bool {
            return pair.first == name;
        };

        return std::find_if(headers.begin(), headers.end(), pred) != headers.end();
    }
}

template <unsigned int N, typename precision>
template <typename T>
inline std::optional<std::shared_ptr<const Column>> TensorField<N, precision>::getMetaData() const {
    if constexpr (std::is_base_of_v<attributes::AttributeBase, T>) {
        const auto& headers = metaData_->getHeaders();
        const auto name = std::string(T::identifier);
        auto pred = [name](const std::pair<std::string, const DataFormatBase*>& pair) -> bool {
            return pair.first == name;
        };

        if (std::find_if(headers.begin(), headers.end(), pred) != headers.end()) {
            return metaData_->getColumn(name);
        }

        return std::nullopt;
    }
}

template <unsigned int N, typename precision>
template <typename T>
inline const std::vector<typename T::value_type>& TensorField<N, precision>::getMetaDataContainer()
    const {
    if constexpr (std::is_base_of_v<attributes::AttributeBase, T>) {
        auto column = *this->getMetaData<T>();
        auto bufferRAM = std::dynamic_pointer_cast<const Buffer<T::value_type>>(column->getBuffer())
                             ->getRAMRepresentation();

        return bufferRAM->getDataContainer();
    }
}
}  // namespace inviwo