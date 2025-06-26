/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AZEROTHCORE_ITEM_RANDOM_ATTRIBUTES_H
#define AZEROTHCORE_ITEM_RANDOM_ATTRIBUTES_H

#include "Common.h"
#include "DatabaseEnv.h"
#include "ItemTemplate.h"
#include <unordered_map>
#include <vector>

class Item;
class Player;

// 物品来源类型定义
enum ItemSourceType : uint32
{
    ITEM_SOURCE_LOOT = 1,      // 怪物掉落
    ITEM_SOURCE_QUEST = 2,     // 任务奖励
    ITEM_SOURCE_VENDOR = 3,    // 商人售卖
    ITEM_SOURCE_MAIL = 4,      // 邮件获得
    ITEM_SOURCE_CHEST = 5,     // 宝箱
    ITEM_SOURCE_EVENT = 6,     // 事件奖励
    ITEM_SOURCE_CRAFT = 7,     // 制造
    ITEM_SOURCE_OTHER = 8      // 其他来源
};

// 属性类型结构体
struct ItemAttributeType
{
    uint8 id;
    std::string name;
    std::string displayName;
    uint8 category;
    float baseMultiplier;
    float qualityMultiplier;
    uint8 minQuality;
    uint8 maxQuality;
    uint32 weight;
    bool enabled;
};

// 随机属性结构体
struct ItemRandomAttribute
{
    uint32 id;
    ObjectGuid::LowType itemGuid;
    uint8 attributeType;
    int32 attributeValue;
    uint8 attributeQuality;
    float randomMultiplier;
    time_t createdAt;
};

// 随机属性管理器类
class ItemRandomAttributesMgr
{
public:
    static ItemRandomAttributesMgr* instance();
    
    // 初始化函数
    void LoadAttributeTypes();
    void LoadItemAttributes();
    
    // 核心功能函数
    bool GenerateRandomAttributes(Item* item, uint32 sourceType = ITEM_SOURCE_OTHER);
    std::vector<ItemRandomAttribute> GetItemAttributes(ObjectGuid::LowType itemGuid);
    void SaveItemAttribute(const ItemRandomAttribute& attr);
    void DeleteItemAttributes(ObjectGuid::LowType itemGuid);
    
    // 辅助函数
    std::string GetAttributeDisplayText(const ItemRandomAttribute& attr);
    int32 CalculateAttributeValue(uint8 attributeType, uint32 itemLevel, uint8 itemQuality);
    bool ShouldGenerateRandomAttributes(Item* item, uint32 sourceType);
    
    // 配置相关
    void LoadConfigSettings();
    void SetEnabled(bool enabled) { _enabled = enabled; }
    bool IsEnabled() const { return _enabled; }
    
    void SetRandomMultiplierRange(float min, float max) 
    { 
        _randomMultiplierMin = min; 
        _randomMultiplierMax = max; 
    }
    
    void SetAttributeValueMultiplier(float multiplier) { _attributeValueMultiplier = multiplier; }
    void SetMaxAttributesPerItem(uint8 max) { _maxAttributesPerItem = max; }

private:
    ItemRandomAttributesMgr() = default;
    ~ItemRandomAttributesMgr() = default;
    
    // 私有辅助函数
    uint8 GetRandomAttributeCount(uint8 itemQuality);
    uint8 SelectRandomAttributeType(uint8 itemQuality, uint8 itemClass, uint8 itemSubClass);
    bool IsAttributeCompatible(uint8 attributeType, uint8 itemClass, uint8 itemSubClass);
    float GetRandomMultiplier();
    
    // 成员变量
    std::unordered_map<uint8, ItemAttributeType> _attributeTypes;
    std::unordered_map<ObjectGuid::LowType, std::vector<ItemRandomAttribute>> _itemAttributes;
    
    // 配置变量
    bool _enabled = true;
    float _randomMultiplierMin = 0.5f;
    float _randomMultiplierMax = 1.5f;
    float _attributeValueMultiplier = 1.0f;
    uint8 _maxAttributesPerItem = 5;
};

#define sItemRandomAttributesMgr ItemRandomAttributesMgr::instance()

#endif // AZEROTHCORE_ITEM_RANDOM_ATTRIBUTES_H 