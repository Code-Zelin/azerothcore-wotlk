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

#include "ItemRandomAttributes.h"
#include "Item.h"
#include "Player.h"
#include "Log.h"
#include "DatabaseEnv.h"
#include "CharacterDatabase.h"
#include "WorldDatabase.h"
#include "Random.h"
#include "Config.h"
#include <sstream>

ItemRandomAttributesMgr* ItemRandomAttributesMgr::instance()
{
    static ItemRandomAttributesMgr instance;
    return &instance;
}

void ItemRandomAttributesMgr::LoadAttributeTypes()
{
    uint32 oldMSTime = getMSTime();
    
    _attributeTypes.clear();
    
    QueryResult result = WorldDatabase.Query("SELECT id, name, display_name, category, base_multiplier, quality_multiplier, min_quality, max_quality, weight, enabled FROM item_attribute_types WHERE enabled = 1 ORDER BY id");
    if (!result)
    {
        LOG_WARN("module", ">> Loaded 0 item attribute types. DB table `item_attribute_types` is empty.");
        return;
    }
    
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        
        ItemAttributeType attrType;
        attrType.id = fields[0].Get<uint8>();
        attrType.name = fields[1].Get<std::string>();
        attrType.displayName = fields[2].Get<std::string>();
        attrType.category = fields[3].Get<uint8>();
        attrType.baseMultiplier = fields[4].Get<float>();
        attrType.qualityMultiplier = fields[5].Get<float>();
        attrType.minQuality = fields[6].Get<uint8>();
        attrType.maxQuality = fields[7].Get<uint8>();
        attrType.weight = fields[8].Get<uint32>();
        attrType.enabled = fields[9].Get<bool>();
        
        _attributeTypes[attrType.id] = attrType;
        ++count;
    } while (result->NextRow());
    
    LOG_INFO("module", ">> Loaded {} item attribute types in {} ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void ItemRandomAttributesMgr::LoadItemAttributes()
{
    uint32 oldMSTime = getMSTime();
    
    _itemAttributes.clear();
    
    QueryResult result = CharacterDatabase.Query("SELECT id, item_guid, attribute_type, attribute_value, attribute_quality, random_multiplier, created_at FROM item_random_attributes");
    if (!result)
    {
        LOG_WARN("module", ">> Loaded 0 item random attributes. DB table `item_random_attributes` is empty.");
        return;
    }
    
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        
        ItemRandomAttribute attr;
        attr.id = fields[0].Get<uint32>();
        attr.itemGuid = fields[1].Get<ObjectGuid::LowType>();
        attr.attributeType = fields[2].Get<uint8>();
        attr.attributeValue = fields[3].Get<int32>();
        attr.attributeQuality = fields[4].Get<uint8>();
        attr.randomMultiplier = fields[5].Get<float>();
        attr.createdAt = fields[6].Get<time_t>();
        
        _itemAttributes[attr.itemGuid].push_back(attr);
        ++count;
    } while (result->NextRow());
    
    LOG_INFO("module", ">> Loaded {} item random attributes in {} ms", count, GetMSTimeDiffToNow(oldMSTime));
}

bool ItemRandomAttributesMgr::ShouldGenerateRandomAttributes(Item* item, uint32 sourceType)
{
    if (!item || !_enabled)
        return false;
        
    ItemTemplate const* proto = item->GetTemplate();
    if (!proto)
        return false;
        
    // 只有装备类物品才能有随机属性
    if (proto->Class != ITEM_CLASS_ARMOR && proto->Class != ITEM_CLASS_WEAPON)
        return false;
        
    // 排除邮件和商人来源的装备
    switch (sourceType)
    {
        case ITEM_SOURCE_MAIL:        // 邮件来源
        case ITEM_SOURCE_VENDOR:      // 商人来源
            return false;
        case ITEM_SOURCE_LOOT:        // 怪物掉落
        case ITEM_SOURCE_QUEST:       // 任务奖励
        case ITEM_SOURCE_CHEST:       // 宝箱
        case ITEM_SOURCE_EVENT:       // 事件奖励
        case ITEM_SOURCE_CRAFT:       // 制造
        default:
            return true;
    }
}

uint8 ItemRandomAttributesMgr::GetRandomAttributeCount(uint8 itemQuality)
{
    switch (itemQuality)
    {
        case ITEM_QUALITY_UNCOMMON:   // 优秀
            return urand(0, 1);
        case ITEM_QUALITY_RARE:       // 精良
            return urand(0, 2);
        case ITEM_QUALITY_EPIC:       // 史诗
            return urand(1, 3);
        case ITEM_QUALITY_LEGENDARY:  // 传说
            return urand(2, 5);
        default:
            return 0;
    }
}

uint8 ItemRandomAttributesMgr::SelectRandomAttributeType(uint8 itemQuality, uint8 itemClass, uint8 itemSubClass)
{
    std::vector<uint8> availableTypes;
    std::vector<uint32> weights;
    
    for (auto const& pair : _attributeTypes)
    {
        ItemAttributeType const& attrType = pair.second;
        
        // 检查品质范围
        if (itemQuality < attrType.minQuality || itemQuality > attrType.maxQuality)
            continue;
            
        // 检查兼容性
        if (!IsAttributeCompatible(attrType.id, itemClass, itemSubClass))
            continue;
            
        availableTypes.push_back(attrType.id);
        weights.push_back(attrType.weight);
    }
    
    if (availableTypes.empty())
        return 0;
    
    // 根据权重随机选择
    uint32 totalWeight = 0;
    for (uint32 weight : weights)
        totalWeight += weight;
    
    uint32 randomWeight = urand(1, totalWeight);
    uint32 currentWeight = 0;
    
    for (size_t i = 0; i < availableTypes.size(); ++i)
    {
        currentWeight += weights[i];
        if (randomWeight <= currentWeight)
            return availableTypes[i];
    }
    
    return availableTypes[0]; // 兜底
}

bool ItemRandomAttributesMgr::IsAttributeCompatible(uint8 attributeType, uint8 itemClass, uint8 itemSubClass)
{
    // 这里可以根据物品类型和属性类型的兼容性进行判断
    // 例如：武器不能有护甲值，护甲不能有武器伤害等
    
    auto it = _attributeTypes.find(attributeType);
    if (it == _attributeTypes.end())
        return false;
    
    ItemAttributeType const& attrType = it->second;
    
    // 基础属性对所有装备都兼容
    if (attrType.category == 1)
        return true;
    
    // 伤害属性主要适用于武器
    if (attrType.category == 2)
        return itemClass == ITEM_CLASS_WEAPON;
    
    // 防御属性主要适用于护甲
    if (attrType.category == 3)
        return itemClass == ITEM_CLASS_ARMOR;
    
    // 攻击属性适用于武器和部分护甲
    if (attrType.category == 4)
        return itemClass == ITEM_CLASS_WEAPON || (itemClass == ITEM_CLASS_ARMOR && itemSubClass != ITEM_SUBCLASS_ARMOR_SHIELD);
    
    // 法术属性适用于所有装备
    if (attrType.category == 5)
        return true;
    
    // 恢复属性适用于所有装备
    if (attrType.category == 6)
        return true;
    
    // 特殊属性适用于所有装备
    if (attrType.category == 7)
        return true;
    
    return true; // 默认兼容
}

float ItemRandomAttributesMgr::GetRandomMultiplier()
{
    return frand(_randomMultiplierMin, _randomMultiplierMax);
}

int32 ItemRandomAttributesMgr::CalculateAttributeValue(uint8 attributeType, uint32 itemLevel, uint8 itemQuality)
{
    auto it = _attributeTypes.find(attributeType);
    if (it == _attributeTypes.end())
        return 0;
        
    ItemAttributeType const& attrType = it->second;
    
    // 特殊处理移动速度属性
    if (attrType.name == "MOVEMENT_SPEED")
    {
        // 移动速度固定为1-3%
        float randomPercent = frand(1.0f, 3.0f);
        return int32(randomPercent * 100); // 转换为整数百分比
    }
    
    // 其他属性的正常计算
    float baseValue = itemLevel * attrType.baseMultiplier;
    float qualityBonus = (itemQuality - 1) * attrType.qualityMultiplier;
    float totalBaseValue = baseValue + qualityBonus;
    
    // 生成随机倍数
    float randomMultiplier = GetRandomMultiplier();
    
    // 应用随机倍数和全局倍数
    float finalValue = totalBaseValue * randomMultiplier * _attributeValueMultiplier;
    
    return int32(finalValue);
}

bool ItemRandomAttributesMgr::GenerateRandomAttributes(Item* item, uint32 sourceType)
{
    if (!ShouldGenerateRandomAttributes(item, sourceType))
        return false;
    
    ItemTemplate const* proto = item->GetTemplate();
    if (!proto)
        return false;
    
    uint8 attributeCount = GetRandomAttributeCount(proto->Quality);
    if (attributeCount == 0)
        return false;
    
    // 限制最大属性数量
    if (attributeCount > _maxAttributesPerItem)
        attributeCount = _maxAttributesPerItem;
    
    std::vector<uint8> selectedTypes;
    
    // 生成随机属性
    for (uint8 i = 0; i < attributeCount; ++i)
    {
        uint8 attributeType = SelectRandomAttributeType(proto->Quality, proto->Class, proto->SubClass);
        if (attributeType == 0)
            continue;
        
        // 避免重复属性
        if (std::find(selectedTypes.begin(), selectedTypes.end(), attributeType) != selectedTypes.end())
            continue;
        
        selectedTypes.push_back(attributeType);
        
        // 计算属性值
        int32 attributeValue = CalculateAttributeValue(attributeType, proto->ItemLevel, proto->Quality);
        if (attributeValue == 0)
            continue;
        
        // 创建随机属性记录
        ItemRandomAttribute attr;
        attr.itemGuid = item->GetGUID().GetCounter();
        attr.attributeType = attributeType;
        attr.attributeValue = attributeValue;
        attr.attributeQuality = proto->Quality;
        attr.randomMultiplier = GetRandomMultiplier();
        attr.createdAt = time(nullptr);
        
        // 保存到数据库
        SaveItemAttribute(attr);
        
        // 添加到内存缓存
        _itemAttributes[attr.itemGuid].push_back(attr);
    }
    
    return !selectedTypes.empty();
}

std::vector<ItemRandomAttribute> ItemRandomAttributesMgr::GetItemAttributes(ObjectGuid::LowType itemGuid)
{
    auto it = _itemAttributes.find(itemGuid);
    if (it != _itemAttributes.end())
        return it->second;
    
    return std::vector<ItemRandomAttribute>();
}

void ItemRandomAttributesMgr::SaveItemAttribute(const ItemRandomAttribute& attr)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_ITEM_RANDOM_ATTRIBUTE);
    stmt->SetData(0, attr.itemGuid);
    stmt->SetData(1, attr.attributeType);
    stmt->SetData(2, attr.attributeValue);
    stmt->SetData(3, attr.attributeQuality);
    stmt->SetData(4, attr.randomMultiplier);
    CharacterDatabase.Execute(stmt);
}

void ItemRandomAttributesMgr::DeleteItemAttributes(ObjectGuid::LowType itemGuid)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_ITEM_RANDOM_ATTRIBUTES);
    stmt->SetData(0, itemGuid);
    CharacterDatabase.Execute(stmt);
    
    _itemAttributes.erase(itemGuid);
}

void ItemRandomAttributesMgr::LoadConfigSettings()
{
    _enabled = sConfigMgr->GetOption<bool>("ItemRandomAttributes.Enabled", true);
    _randomMultiplierMin = sConfigMgr->GetOption<float>("ItemRandomAttributes.RandomMultiplierMin", 0.5f);
    _randomMultiplierMax = sConfigMgr->GetOption<float>("ItemRandomAttributes.RandomMultiplierMax", 1.5f);
    _attributeValueMultiplier = sConfigMgr->GetOption<float>("ItemRandomAttributes.AttributeValueMultiplier", 1.0f);
    _maxAttributesPerItem = sConfigMgr->GetOption<uint8>("ItemRandomAttributes.MaxAttributesPerItem", 5);
    
    // 验证配置值的合理性
    if (_randomMultiplierMin > _randomMultiplierMax)
    {
        LOG_ERROR("module", "ItemRandomAttributes: RandomMultiplierMin ({}) cannot be greater than RandomMultiplierMax ({}). Using defaults.", _randomMultiplierMin, _randomMultiplierMax);
        _randomMultiplierMin = 0.5f;
        _randomMultiplierMax = 1.5f;
    }
    
    if (_maxAttributesPerItem > 10)
    {
        LOG_ERROR("module", "ItemRandomAttributes: MaxAttributesPerItem ({}) is too high. Using maximum value of 10.", _maxAttributesPerItem);
        _maxAttributesPerItem = 10;
    }
    
    LOG_INFO("module", "ItemRandomAttributes: Loaded configuration - Enabled: {}, MultiplierRange: {}-{}, GlobalMultiplier: {}, MaxAttributes: {}", 
             _enabled, _randomMultiplierMin, _randomMultiplierMax, _attributeValueMultiplier, _maxAttributesPerItem);
}

std::string ItemRandomAttributesMgr::GetAttributeDisplayText(const ItemRandomAttribute& attr)
{
    auto it = _attributeTypes.find(attr.attributeType);
    if (it == _attributeTypes.end())
        return "未知属性";
    
    ItemAttributeType const& attrType = it->second;
    std::stringstream ss;
    
    // 特殊处理移动速度
    if (attrType.name == "MOVEMENT_SPEED")
    {
        float percent = attr.attributeValue / 100.0f;
        ss << attrType.displayName << " +" << std::fixed << std::setprecision(1) << percent << "%";
    }
    else
    {
        ss << attrType.displayName << " +" << attr.attributeValue;
    }
    
    return ss.str();
} 