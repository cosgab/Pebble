module.exports = [
  {
    "type": "heading",
    "defaultValue": "GC Watch Config"
  },
  {
    "type": "text",
    "defaultValue": "Choose colours."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Background Colours"
      },
      {
        "type": "color",
        "messageKey": "backgroundColor",
        "defaultValue": "0x000000",
        "label": "Background Bluetooth OK"
      },
      {
        "type": "color",
        "messageKey": "backgroundBTColor",
        "defaultValue": "0xFFFFFF",
        "label": "Background Bluetooth KO"
      },
      {
        "type": "color",
        "messageKey": "quadrantColor",
        "defaultValue": "0xFFFFFF",
        "label": "Inner night"
      },
      {
        "type": "color",
        "messageKey": "innerFillColor",
        "defaultValue": "0xFFFFFF",
        "label": "Inner daylight"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Objects Colours"
      },
      {
        "type": "color",
        "messageKey": "textColor",
        "defaultValue": "0xFFFFFF",
        "label": "Date and steps counter"
      },
      {
        "type": "color",
        "messageKey": "batteryColor",
        "defaultValue": "0xFFFFFF",
        "label": "Battery"
      },
      {
        "type": "color",
        "messageKey": "hourColor",
        "defaultValue": "0xFFFFFF",
        "label": "Text time"
      },

      {
        "type": "color",
        "messageKey": "hourhandColor",
        "defaultValue": "0xFFFFFF",
        "label": "Hours hand"
      },
      {
        "type": "color",
        "messageKey": "minhandColor",
        "defaultValue": "0xFFFFFF",
        "label": "Minutes hand"
      },
      {
        "type": "color",
        "messageKey": "handColor24",
        "defaultValue": "0xFFFFFF",
        "label": "Day time hand"
      }
    ]
  },  
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Open Weather"
      },
      {
        "type": "toggle",
        "messageKey": "weatherView",
        "label": "Enable weather",
        "defaultValue": true
      },
      {
        "type": "input",
        "messageKey": "API_key",
        "defaultValue": "",
        "label": "Open Weather APY Key"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "save"
  }
];