"use strict";
"require view";
"require rpc";
"require form";
"require uci";
"require ui";
"require dom";
"require tools.widgets as widgets";
"require network";

//////////////////////////
// defines
//////////////////////////

const ROLE_DISABLED = 0;
const ROLE_CONTROLLER = 1;
const ROLE_AGENT = 2;

let vars = {};
vars.ssid_chosen = {};
vars.vlan_names = [];
vars.pcpvs_2 = {};

let reloadVars = () => {
  vars.vlan_names = [];
  vars.ssid_chosen = {};
  vars.pcpvs_2 = {};
  uci.sections("rtkmap", "vlan", (vlan, section_id) => {
    vars.vlan_names.push(vlan.name);
    vlan.ssids.map((s) => {
      vars.ssid_chosen[s] = 1;
    })
    vars.pcpvs_2[section_id] = [];
    vars.pcpvs_2[section_id].push(...vlan.ssids);
  });
}

/////////////////////////////////
// helper functions: renderers, handlers
/////////////////////////////////
let CBISSIDSelect = form.ListValue.extend({
  __name__: "CBI.SSIDSelect",

  load: async function (section_id) {
    let data = await network.getWifiDevices();
    this.wifidevices = data;
    this.ssids = data.reduce((prev, curr) => {
      prev.push(
        ...(curr._ubusdata?.dev?.interfaces?.map((x) => ({
          ifname: x.ifname,
          name: x.config?.ssid,
          config: x.config,
        })) ?? [])
      );
      return prev;
    }, []);
    this.ssids = [...new Set(this.ssids)];
    this.ssids = this.ssids.filter((s) => {
      return s.config.multi_ap !== 1;
    });
    return this.super("load", section_id);
  },

  filter: function (section_id, value) {
    return true;
  },

  renderWidget: function (section_id, option_index, cfgvalue) {
    let values = L.toArray(cfgvalue != null ? cfgvalue : this.default),
      choices = {},
      checked = {},
      order = [];
    for (let ssid of this.ssids) {
      let vlan_name = ssid.name;
      if (!this.filter(section_id, vlan_name)) continue;
      let item = E([
        E("img", {
          title: vlan_name,
          src: L.resource("icons/wifi.png"),
        }),
        E("span", { class: "hide-open" }, [vlan_name]),
        E("span", { class: "hide-close" }, [vlan_name]),
      ]);

      choices[vlan_name] = item;
      order.push(vlan_name);
    }
    var widget = new ui.Dropdown(this.multiple ? values : values[0], choices, {
      id: this.cbid(section_id),
      sort: order,
      multiple: this.multiple,
      optional: this.optional || this.rmempty,
      disabled: this.readonly != null ? this.readonly : this.map.readonly,
      select_placeholder: E("em", _("unspecified")),
      display_items: this.display_size || this.size || 3,
      dropdown_items: this.dropdown_size || this.size || 5,
      validate: L.bind(this.validate, this, section_id),
      create: !this.nocreate,
      create_markup:
        "" +
        '<li data-value="{{value}}">' +
        '<img title="' +
        _("Custom Interface") +
        ': &quot;{{value}}&quot;" src="' +
        L.resource("icons/ethernet_disabled.png") +
        '" />' +
        '<span class="hide-open">{{value}}</span>' +
        '<span class="hide-close">' +
        _("Custom Interface") +
        ': "{{value}}"</span>' +
        "</li>",
    });

    return widget.render();
  }
});

let CBIVLANSelect = form.ListValue.extend({
  __name__: "CBI.VLANSelect",

  load: async function (section_id) {
    await uci.changes();
    await uci.load("rtkmap");
    this.vlans = [];
    uci.sections("rtkmap", "vlan", (vlan, section_id) => {
      this.vlans.push(vlan);
    });
    return this.super("load", section_id);
  },

  filter: function (section_id, value) {
    return true;
  },

  renderWidget: function (section_id, option_index, cfgvalue) {
    let values = L.toArray(cfgvalue != null ? cfgvalue : this.default),
      choices = {},
      checked = {},
      order = [];
    for (let vlan of this.vlans) {
      let vlan_name = vlan.name;
      if (!this.filter(section_id, vlan_name)) continue;
      let item = E([
        E("img", {
          title: vlan_name,
          src: L.resource("icons/wifi.png"),
        }),
        E("span", { class: "hide-open" }, [vlan_name]),
        E("span", { class: "hide-close" }, [vlan_name]),
      ]);

      choices[vlan_name] = item;
      order.push(vlan_name);
    }
    var widget = new ui.Dropdown(this.multiple ? values : values[0], choices, {
      id: this.cbid(section_id),
      sort: order,
      multiple: this.multiple,
      optional: this.optional || this.rmempty,
      disabled: this.readonly != null ? this.readonly : this.map.readonly,
      select_placeholder: E("em", _("unspecified")),
      display_items: this.display_size || this.size || 3,
      dropdown_items: this.dropdown_size || this.size || 5,
      validate: L.bind(this.validate, this, section_id),
      create: !this.nocreate,
      create_markup:
        "" +
        '<li data-value="{{value}}">' +
        '<img title="' +
        _("Custom Interface") +
        ': &quot;{{value}}&quot;" src="' +
        L.resource("icons/ethernet_disabled.png") +
        '" />' +
        '<span class="hide-open">{{value}}</span>' +
        '<span class="hide-close">' +
        _("Custom Interface") +
        ': "{{value}}"</span>' +
        "</li>",
    });

    return widget.render();
  }
});


/////////////////////////////////
// renderers
/////////////////////////////////
let genVLANSettingsView = async function () {
  let m, s1, s2, o;
  m = new form.Map("rtkmap", _("VLAN"));

  s1 = m.section(form.TypedSection, "rtk_map", _("General Settings"));
  s1.anonymous = true;
  s1.redraw = function () {
    return this.load().then(L.bind(this.render, this));
  };

  o = s1.option(
    form.Flag,
    "enable_vlan",
    _("Enable VLAN")
  );
  o.default = '0';
  o.rmempty = false;

  o = s1.option(CBIVLANSelect, "primary_vlan_name", _("Primary VLAN Name"));
  o.depends("enable_vlan", "1");

  s2 = m.section(form.GridSection, "vlan", _("VLANs"));
  s2.addremove = true;
  s2.anonymous = true;
  s2.redraw = function () {
    return this.load().then(L.bind(this.render, this));
  };
  s2.handleRemove = async function(section_id, ev) {
    let config_name = this.uciconfig || this.map.config;
    let sec = this.map.data.get(config_name, section_id);
    let ssids = sec.ssids;
    for (let v of ssids) {
      if (--(vars.ssid_chosen[v]) === 0) delete vars.ssid_chosen[v];
    }
    delete vars.pcpvs_2[section_id];
    let name_idx = vars.vlan_names.indexOf(sec.name);
    if (name_idx > -1) vars.vlan_names.splice(name_idx, 1);

    this.map.data.remove(config_name, section_id);
    await s1.parse().then(function () { s1.redraw(); });
    await s2.parse().then(function () { s2.redraw(); });
    return this.map.save(null, true);
  };
  s2.handleModalSave = async function(/* ... */) {
    let ans = this.super('handleModalSave', arguments);
    await uci.changes();
    await uci.load("rtkmap");
    reloadVars();
    await s1.parse().then(function () { s1.redraw(); });
    await s2.parse().then(function () { s2.redraw(); });
    return ans;
  }

  o = s2.option(form.Value, "name", _("Name"), _("Name of VLAN"));
  o.placeholder = _("Unnamed VLAN");
  o.validate = function (section_id, value) {
    if (Object.values(this.data)[0] === value) return true;
    if (value == "") return _("Please input name");
    if (vars.vlan_names.includes(value)) return _("Name already taken");
    return true;
  };

  o = s2.option(CBISSIDSelect, "ssids", _("SSIDs"), _("SSIDs to include"));
  o.rmempty = true;
  o.multiple = true;
  o.filter = function (section_id, ssid) {
    if (vars.pcpvs_2[section_id]?.includes(ssid)) return true;
    if (vars.ssid_chosen[ssid] !== undefined) return false;
    return true;
  };
  o.onchange = async function (ev, section_id, values) {
    let pcpvs = vars.pcpvs_2[section_id] ?? [];
    for (let v of values) {
      if (pcpvs.includes(v)) continue;
      if (vars.ssid_chosen[v] === undefined) vars.ssid_chosen[v] = 0;
      vars.ssid_chosen[v]++;
    }
    for (let v of pcpvs) {
      if (values.includes(v)) continue;
      if (--(vars.ssid_chosen[v]) === 0) delete vars.ssid_chosen[v];
    }
    vars.pcpvs_2[section_id] = values;
    await s1.parse().then(function () { s1.redraw(); });
    await s2.parse().then(function () { s2.redraw(); });
  };

  return m.render();
};

return view.extend({
  load: async function () {
    await uci.changes();
    await uci.load("rtkmap");
    let role = uci.get("rtkmap", "map", "controller");
    if (role != ROLE_CONTROLLER) return;
    reloadVars();
  },

  render: async function () {
    let role = uci.get("rtkmap", "map", "controller");
    if (role != ROLE_CONTROLLER) return E("h3", _("VLAN function is available in Controller mode only."));
    return genVLANSettingsView();
  },
});
