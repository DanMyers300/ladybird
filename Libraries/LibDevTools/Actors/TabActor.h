/*
 * Copyright (c) 2025, Tim Flynn <trflynn89@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/NonnullRefPtr.h>
#include <LibDevTools/Actor.h>

namespace DevTools {

struct TabDescription {
    u64 id { 0 };
    ByteString title;
    ByteString url;
};

class TabActor final : public Actor {
public:
    static constexpr auto base_name = "tab"sv;

    static NonnullRefPtr<TabActor> create(DevToolsServer&, ByteString name, TabDescription);
    virtual ~TabActor() override;

    virtual void handle_message(StringView type, JsonObject const&) override;

    TabDescription const& description() const { return m_description; }
    JsonObject serialize_description() const;

private:
    TabActor(DevToolsServer&, ByteString name, TabDescription);

    TabDescription m_description;
    WeakPtr<WatcherActor> m_watcher;
};

}
