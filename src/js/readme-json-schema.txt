==================
Input QueryString:
==========
Parameter: 		title
Description: 	A text to show as the title of the configuration page.
				If title is blank, or is missing, the value is defaulted to "Configuration"

==========
Parameter: 		fields
Description: 	A JSON formatted string that defines the fields to show on the configuration page.
	 			If fields is blank, is missing, or does not follow the schema below, the configuration page will report an error.
Schema:
	[
		{
			'caption':'string',
			'key':'string',
			'initialValue':'string',
			'type':'radiobutton|textbox|checkbox',
			'values':
			[
				{
					'text':'string',
					'value':'string'
				},
				{
					'text':'string',
					...
				},
				...
			]
		},
		{
			'caption':'string',
			'key':'string',
			...
		},
		...
	]
	
Schema description:
- Each field will be an array of 5 name-value pair items.
- Below are the valid items:
  "caption": 		A text to show at the heading of the field.
  					Optional item.
  					If this item is blank or missing, the value of the key will be used instead.
  "key": 			A text used to identify the field. 
  					Required item.
					This should be one of the appKeys defined on the appinfo.json file.
  					If this item is blank or missing, an error will be thrown on the configuration page.
  					This item will be returned when the configuration page closes.
  "type":			A text used to determine what control will be displayed to represent the field on the configuration page.
  					Optional item.
  					Valid values are: checkbox, textbox, radiobutton.
					If this item is blank, missing, or has an invalid text, the value will be defaulted to textbox.
  "initialValue":	A text used to set the selected value of the field.
  					Optional item.
  					If "type" is "textbox", the text on the textbox will contain the value of this item.
					If "type" is "checkbox", the checkbox will be checked if the underlying value of the checkbox matches the value of this item.
					If "type" is "radiobutton", the radiobutton that has an underlying value which matches the value of this item will be selected.
					If this item is blank, or missing, or has an invalid value, it will be ignored.
  "values":			An array of 2 name-value pairs items.
  					This array determines all the possible values for the field.
					If "type" is "textbox", this item is ignored.
					If "type" is "checkbox", only the first item is used to represent the underlying value of the checkbox.
					If "type" is "checkbox", and the "values" item is blank or missing, the checkbox's underlying value will be defaulted to "1".
					If "type" is "radiobutton", this item is required.
					If "type" is "radiobutton", each item in the array will be represented by a radiobutton and will be that radiobutton's underlying value.
					Below are the valid items:
					"text":		A text to show beside the radiobutton.
								Optional item.
								If "type" is "textbox" or "checkbox", this item is ignored.
								If this item is blank or missing, the value will be defaulted to the value of the "value" item.
					"value":	A text to represent the value of the field.
								If "type" is "textbox", this item is ignored.
								If "type" is "checkbox", this item is required (although only the first in the array of "values" will be checked).
								If "type" is "radiobutton", this item is required.


==================
Output Parameters:
==========
Description:	When the configuration page closes, a JSON formatted text will be returned.
				This text will have an array of name-value pair items.
Schema:
	pebbjs://close#{'action':'save|cancel','[field1]':'string','[field2]':'string',...}

Schema description:
- Each field shown on the configuration page will have it's underlying value returned when the field has a value or is selected.
- This means that if for example, there is a textbox on the configuration page but no text is entered, the field represented by the textbox will not be returned.
- Similarly, if there is a checkbox but it is not checked, the field represented by that checkbox will not be returned.
- The returned value will be accompanied by the id of the field in the form of a name-value pair.
- The id of the field is defined by the JSON formatted text passed to the "fields" QueryString parameter when the configuration page initially loaded.
- Below are the valid items:
  "action":		A text used to determine what button is clicked on the configuration page.
				Required item.
				Valid values are: save, cancel.
				Typical usage is to persist the changes to the phone or watch when "action" is "save".
  "[field]":	A text that represents the selected value of a field on the configuration page.
				Optional item.
				The "[field]" item is a placeholder, it will be replaced by the field's "key".
				If the field's "type" is "textbox", the value of this item is from the text on the textbox.
				If the field's "type" is "checkbox", the value of this item is from underlying value of the checkbox, if the checkbox is checked.
				If the field's "type" is "radiobutton", the value of this item is from the underlying value of the selected radiobutton.
				
==================
Example:
==========
Scenario:	Show a checkbox on the configuration page, titled "Hourly Vibrate".
			The checkbox will have its underlying value defaulted to 1 (see schema).
			And since the "initialValue" is "1", the checkbox is initially checked when the configuration page loads.
			
fields =	[ 
				{
					'caption':'Hourly Vibrate',
					'key':'hourly_vibrate',
					'initialValue':'1',
					'type':'checkbox'
				}
			]

==========
Scenario:	Show a textbox on the configuration page, titled "Area Code".
			Even though the type is invalid, the field will still be shown as a textbox (see schema).
			
fields =	[ 
				{
					'caption':'Area Code',
					'key':'area_code',
					'type':'this will be defaulted to textbox anyway'
				}
			]
			
==========
Scenario:	Show a checkbox on the configuration page, titled "BT Status Vibrate".
			When the Save button is clicked and the checkbox is checked, the configuration page will return:
				pebbjs://close#{'action':'save','bt_status':'ON'}
			
fields =	[ 
				{
					'caption':'BT Status Vibrate',
					'key':'bt_status',
					'type':'checkbox',
					'values':
					[
						{
							'value':'ON'
						}
					]
				}
			]
			
==========
Scenario:	Show 2 radiobuttons on the configuration page, titled "Display Options".
			When the Cancel button is clicked even when any radiobutton is selected, the configuration page will return:
				pebbjs://close#{'action':'cancel'}
			
fields =	[ 
				{
					'caption':'Display Options',
					'key':'options',
					'type':'radiobutton',
					'values':
					[
						{
							'text':'White on Black',
							'value':'0'
						},
						{
							'text':'Black on White',
							'value':'1'
						}
					]
				}
			]

==========
Scenario:	Show 3 radiobuttons on the configuration page, titled "ScrollOptions".
			The title is defaulted from the "key" item.
						
fields =	[ 
				{
					'key':'ScrollOptions',
					'type':'radiobutton',
					'values':
					[
						{
							'text':'Left to Right',
							'value':'LTR'
						},
						{
							'text':'Right to Left',
							'value':'RTL'
						},
						{
							'value':'None'
						},
					]
				}
			]

==========
Scenario:	Show a checkbox and a radiobutton on the configuration page.
			If the Save button is clicked, and both fields has a value, the configuration page will return:
				pebbjs://close#{'action':'save','ShowDate':'1','DateFormat':'2'}
									
fields =	[ 
				{
					'caption':'Show Date',
					'key':'ShowDate',
					'type':'checkbox'
				},
				{
					'caption':'Date Format',
					'key':'DateFormat',
					'type':'radiobutton',
					'values':
					[
						{
							'text':'MM/dd/yyyy',
							'value':'0'
						},
						{
							'text':'dd/MM/yyyy',
							'value':'1'
						},
						{
							'text':'MMM-dd'
							'value':'2'
						},
					]
				},
			]

==========
Scenario:	Show a checkbox and a textbox on the configuration page.
			If the Save button is clicked, but there are no changes made to the configuration page, it will return:
				pebbjs://close#{'action':'save','Text':'Your text here'}
									
fields =	[ 
				{
					'caption':'Show Text',
					'key':'ShowText',
					'type':'checkbox'
				},
				{
					'key':'Text',
					'initialValue':'Your text here'
				},
			]

